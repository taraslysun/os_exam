#include "read_cfg.hpp"
#include <unordered_map>
#include <boost/asio.hpp>
#include <fstream>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/sync/named_semaphore.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/process.hpp>
#define PORT 8000
#include <iostream>

namespace bip = boost::interprocess;
namespace ba = boost::asio;


class MyMPI {

private:
    int rank;
    int world_size;
    bool use_shared_memory;
    size_t max_message_size = 256;
    std::string config_file;
    std::unordered_map<std::string, std::string> cfg;
    std::vector<std::string> peers;

    bip::shared_memory_object shm;
    bip::mapped_region region;

    std::string hostname;
    int port;
    ba::io_context io_context;

    std::thread io_thread;

    void setup_communication();
    void setup_shared_memory();
    void setup_sockets();

    size_t calculate_offset(int sender_rank, int receiver_rank);

public:
    MyMPI(int argc, char* argv[]);
    ~MyMPI();         
    int get_rank() const;   
    int get_world_size() const;

    void barrier();
    bool test(int request_id);
    void wait(int request_id);


    template <typename T>
    void send(int dest_rank, const T* data, size_t count, std::string dest_hostname = "") {
        if (use_shared_memory) {
            char* shm_data = static_cast<char*>(region.get_address());
            size_t offset = calculate_offset(rank, dest_rank);
            memcpy(shm_data + offset, data, sizeof(T) * count);

            std::string sem_name = "sem_" + std::to_string(rank) + "_to_" + std::to_string(dest_rank);
            bip::named_semaphore sem(bip::open_or_create, sem_name.c_str(), 0);
            sem.post();
        }
        else {
            std::cout << "Sending to " << dest_hostname << ' ' << dest_rank << std::endl;
            ba::ip::tcp::socket socket(io_context);
            ba::ip::tcp::resolver resolver(io_context);
            ba::connect(socket, resolver.resolve(dest_hostname, std::to_string(PORT + dest_rank)));
            ba::write(socket, ba::buffer(data, sizeof(T) * count));

        }
        
    }

    template <typename T>
    void recv(int src_rank, T* buffer, size_t count) {
        if (use_shared_memory) {

            std::string sem_name = "sem_" + std::to_string(src_rank) + "_to_" + std::to_string(rank);
            bip::named_semaphore sem(bip::open_or_create, sem_name.c_str(), 0);
            sem.wait();

            char* shm_data = static_cast<char*>(region.get_address());
            size_t offset = calculate_offset(src_rank, rank);
            memcpy(buffer, shm_data + offset, sizeof(T) * count);

            bip::named_semaphore::remove(sem_name.c_str());
        }
        else {
            ba::ip::tcp::acceptor acceptor(io_context, ba::ip::tcp::endpoint(ba::ip::tcp::v4(), port));
            ba::ip::tcp::socket socket = acceptor.accept();
            std::cout << "Receiving from " << socket.remote_endpoint().address().to_string() << ' ' << src_rank << std::endl;
            ba::read(socket, ba::buffer(buffer, sizeof(T) * count));
        }
    }

};
