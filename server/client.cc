/* CSE 5306: Distributed Systems
 * Project 1
 * Kaesi Manakkal
 */

#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include <fstream>
#include <cstdlib>

#include "project1.grpc.pb.h"


using grpc::Channel;
using grpc::ChannelArguments;
using grpc::ClientContext;
using grpc::Status;

using project1::ImgSearchEngine;
using project1::Query;
using project1::Image;

class Client {
    public:
    Client(std::shared_ptr<Channel> channel) : stub_(ImgSearchEngine::NewStub(channel)) {}

    // Assembles the client's payload, sends it and presents the response back
    // from the server.
    Image MakeQuery(const std::string& query) {
        // Data we are sending to the server.
        Query request;
        request.set_query(query);

        // Container for the data we expect from the server.
        Image img;

        // Context for the client. It could be used to convey extra information to
        // the server and/or tweak certain RPC behaviors.
        ClientContext context;

        // The actual RPC.
        Status status = stub_->MakeQuery(&context, request, &img);

        // Act upon its status.
        if (!status.ok()) {
            std::cout << status.error_code() << ": " << status.error_message()
                    << std::endl;
        }
        return img;
    }

    private:
    std::unique_ptr<ImgSearchEngine::Stub> stub_;
};

void saveImage(std::string filename, std::string bytes, int len) {
    std::ofstream outfile(filename);
    outfile.write(bytes.c_str(), len);
    outfile.close();
}

int main(int argc, char** argv) {
    std::string server_host("localhost:50051"), query(""), outfile("");

    // get host from env variable
    if(const char* env_server_host = std::getenv("SERVER_HOST"))
        server_host = env_server_host;

    int option;
    while((option = getopt(argc, argv, ":hHs:q:o:")) != -1){
        switch(option){
            case 'h': case 'H':
                std::cout << "Usage: " << argv[0] << "[options] -o <outfile> -q <query>" << std::endl \
                      << "   -h                   - display this help" << std::endl \
                      << "   -s host:port         - specify server" << std::endl \
                      << "   -o /path/to/img.jpg  - filename for image" << std::endl \
                      << "   -q query             - keyword to search" << std::endl;
                return 0;
                break;
            case 'o':
                outfile = optarg;
                break;
            case 's':
                server_host = optarg;
                break;
            case 'q':
                query = optarg;
                break;
            case ':':
                std::cout << argv[0] << ": error: option needs value" << std::endl;
                return 1;
            case '?':
                std::cout << argv[0] << ": error: unknown option: " << optopt << std::endl;
                std::cout << argv[0] << ": use -h for help" << std::endl;
                return 1;
        }
    }

    if((query.length() == 0) || (outfile.length() == 0)) {
        std::cout << argv[0] << ": error: must specify query and outfile" << std::endl;
        return 1;
    }

    ChannelArguments args;
    args.SetLoadBalancingPolicyName("round_robin");

    // create grpc client
    Client img_search_client(grpc::CreateCustomChannel(
        server_host, grpc::InsecureChannelCredentials(), args));
    
    // send request to server
    Image img = img_search_client.MakeQuery(query);

    if(img.error() == 200) {
        saveImage(outfile, img.image(), img.len());
        std::cout << "Downloaded image successfully to '" << outfile << "'!" << std::endl;
    } else if (img.error() == 404) {
        std::cout << "No results for: '" << query << "'" << std::endl;
        return 0;
    } else {
        std::cout << "Server error" << std::endl;
        return 0;
    }

    return 0;
} 
