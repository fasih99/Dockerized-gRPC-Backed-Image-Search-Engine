#include <iostream>
#include <string.h>


#include <getopt.h>
#include <unistd.h>
std::string db_dir_path = "./";
std::string listen_addr = "localhost:50051";

int main(int argc, char** argv) {
    int option;
    while((option = getopt(argc, argv, ":hHd:l:")) != -1){
        switch(option){
            case 'h': case 'H':
                std::cout << "Usage: " << argv[0] << "[options]" << std::endl \
                      << "   -h               - display this help" << std::endl \
                      << "   -d /path/to/db   - specify db dir" << std::endl \
                      << "   -l address:port  - listen on address:port" << std::endl;
                return 0;
                break;
            case 'd':
                db_dir_path = optarg;
                if(access(db_dir_path.c_str(), R_OK) != 0) {
                    std::cout << argv[0] << ": error: invalid db dir" << std::endl;
                    return 1;
                }
                break;
            case 'l':
                listen_addr = optarg;
                break;
        }
    }

    std::cout << db_dir_path << std::endl;
    
    return 0;
}