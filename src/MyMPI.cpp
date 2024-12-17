#include "MyMPI.hpp"

MyMPI::MyMPI(int argc, char* argv[]) : resolver(io_context) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <rank> <config_file>" << std::endl;
        exit(1);
    }
    rank = std::stoi(argv[1]);
    std::unordered_map<std::string, std::string> cfg_;
    parse_cfg(argv[2], cfg_);
    cfg = cfg_;
    config_file = argv[2];

    use_shared_memory = cfg["mode"] == "0";
    world_size = std::stoi(cfg["process_count"]);

    setup_communication();
}

MyMPI::~MyMPI() {
    if (use_shared_memory) {
        bip::shared_memory_object::remove(cfg["shared_memory"].c_str());
        bip::named_semaphore::remove("mpi_semaphore");
    }
}

int MyMPI::get_rank() const {
    return rank;
}

int MyMPI::get_world_size() const {
    return world_size;
}

void MyMPI::setup_communication() {
    if (use_shared_memory) {
        setup_shared_memory();
    } else {
        setup_sockets();
    }
}

void MyMPI::setup_shared_memory() {
    const char* shm_name = cfg["shared_memory"].c_str();
    try {
        if (rank == 0) {
            bip::shared_memory_object::remove(shm_name);
            shm = bip::shared_memory_object(bip::create_only, shm_name, bip::read_write);
            shm.truncate(max_message_size * world_size);
        } else {
            shm = bip::shared_memory_object(bip::open_only, shm_name, bip::read_write);
        }
        region = bip::mapped_region(shm, bip::read_write);
    } catch (const std::exception& e) {
        std::cerr << "Error setting up shared memory: " << e.what() << std::endl;
        exit(1);
    }
}

void MyMPI::setup_sockets() {
    try {
        for (int i = 0; i < world_size; ++i) {
            std::string ip_key = "ip_address_" + std::to_string(i);
            peers.push_back(cfg[ip_key]);
        }
        port = std::stoi(cfg["port_base"]); // Get base port from config

        acceptor = std::make_shared<ba::ip::tcp::acceptor>(io_context);
        ba::ip::tcp::endpoint endpoint(ba::ip::tcp::v4(), port + rank);
        acceptor->open(endpoint.protocol());
        acceptor->set_option(ba::ip::tcp::acceptor::reuse_address(true));
        acceptor->bind(endpoint);
        acceptor->listen();

        start_accept();

        io_thread = std::thread([this]() {
            io_context.run();
        });
    } catch (const std::exception& e) {
        std::cerr << "Error setting up sockets: " << e.what() << std::endl;
        exit(1);
    }
}


void MyMPI::start_accept() {
    auto socket = std::make_shared<ba::ip::tcp::socket>(io_context);
    acceptor->async_accept(*socket, [this, socket](boost::system::error_code ec) {
        if (!ec) {
            asyncRead(socket, nullptr);
        } else {
            std::cerr << "Error accepting connection: " << ec.message() << std::endl;
        }
        start_accept(); // Continue accepting connections
    });
}

size_t MyMPI::calculate_offset(int sender_rank, int receiver_rank) {
    return (sender_rank * world_size + receiver_rank) * max_message_size;
}

void MyMPI::barrier() {
    static bip::named_semaphore sem(bip::open_or_create, "mpi_semaphore", 0);

    if (rank == 0) {
        for (int i = 1; i < world_size; ++i) {
            sem.post();
        }
    } else {
        sem.wait();
    }
}
