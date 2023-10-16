/* CSE 5306: Distributed Systems
 * Project 1
 * Kaesi Manakkal
 * Adapted from grpc's hello world example
 */

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <random>
#include <fstream>
#include <string.h>
#include <grpcpp/grpcpp.h>
#include <sys/types.h>
#include <dirent.h>
#include "project1.grpc.pb.h"
#include <thread>
#include <getopt.h>
#include <unistd.h>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using project1::ImgSearchEngine;
using project1::Query;
using project1::Image;

// functions
std::vector<std::string> *listDir(std::string dir_path);     // list files in a directory
std::string searchImage(std::string query, int *len);        // search for and grab image

// global config variables
std::string db_dir_path = "./";                         // default database dir
std::string listen_addr = "0.0.0.0:50051";              // default listening address

// gRPC service protocol class implementation
class ImageSvc final : public ImgSearchEngine::Service {
    // make a search
    Status MakeQuery(ServerContext* context, const Query* request, 
        Image* reply) override {
        int imglen = 0;
        // do search, grab image
        std::string image = searchImage(request->query(), &imglen);

        // tell client about errors
        if(!image.compare("ENODB") || !image.compare("EFILEOPN"))
            reply->set_error(500);
        else if(!image.compare("ENOTFOUND")) reply->set_error(404);
        else reply->set_error(200);

        reply->set_len(imglen);
        reply->set_image(image);
        return Status::OK;
    }
};

// return a list of the files in a directory
std::vector<std::string> *listDir(std::string dir_path) {
    std::vector<std::string> *dirlist = new std::vector<std::string>;
    // open dir
    DIR *dir = opendir(dir_path.c_str());

    // dir does not exist
    if(dir == NULL) {
        delete dirlist; // we don't leak memory
        return NULL;
    }

    // walk through directory
    struct dirent *dir_ent;
    while((dir_ent = readdir(dir)) != NULL) {
        // add dir info to vector
        if((strcmp(dir_ent->d_name, ".") != 0) && 
            (strcmp(dir_ent->d_name, "..") != 0))
            dirlist->push_back(dir_ent->d_name);
    }

    // clean up
    closedir(dir);
    return dirlist;
}

// search db_dir_path for a directory called query
// then open that dir and read a random image
// return that image as a string of bytes
std::string searchImage(std::string query, int *len) {
    // get list of possible queries from db dir
    std::vector<std::string> *ClassList = listDir(db_dir_path);
    std::string imagebytes, newpath;
    std::ifstream img;
    char c;

    // db does not exist
    if(ClassList == NULL)
        return "ENODB";

    // search db for query
    bool exists = false;
    for(auto c: *ClassList) {
        if(c.compare(query) == 0) {
            exists = true;
            break;
        }
    }

    delete ClassList;
    // no results
    if(!exists) return "ENOTFOUND";

    // object class was found; return random image
    newpath = db_dir_path + "/" + query;
    ClassList = listDir(newpath);

    // c++11 random number generation
    std::random_device randomdev;
    std::default_random_engine randengine(randomdev());
    std::uniform_int_distribution<int> uniformdst(0, ClassList->size()-1);

    // get index between 0 and num_classes (ClassList->size())
    int random_index = uniformdst(randengine);

    // open the image file, return contents
    newpath = newpath + "/" + ClassList->at(random_index);

    #ifdef DEBUG
        std::cout << "opening file: " << newpath << std::endl;
    #endif

    // open image as binary, return contents
    img.open(newpath, std::ios_base::in | std::ios_base::binary);
    delete ClassList;
    if(!img) return "EFILEOPN";

    while(img.get(c)) {
        (*len)++;
        imagebytes += c;
    }
    img.close();
    return imagebytes;
}

void RunServer(int n) {
    ImageSvc service;
    ServerBuilder builder;

    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort(listen_addr, grpc::InsecureServerCredentials());

    // Register "service" as the instance through which we'll communicate with clients.
    builder.RegisterService(&service);

    // assemble the server.
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "thread[" << n << "]: server listening on " << listen_addr << std::endl;

    // wait indefinately for server
    server->Wait();
}

int main(int argc, char** argv) {
    int num_threads = 2;                // use two threads by default
    std::vector<std::thread> threads;   // thread vector to keep track of them

    int option;
    while((option = getopt(argc, argv, ":hHt:d:l:")) != -1){
        switch(option) {
            case 'h': case 'H':
                std::cout << "Usage: " << argv[0] << "[options]" << std::endl \
                      << "   -h               - display this help" << std::endl \
                      << "   -t <num>         - specify number of server threads" << std::endl \
                      << "   -d /path/to/db   - specify db dir" << std::endl \
                      << "   -l address:port  - listen on address:port" << std::endl;
                return 0;
                break;
            case 'd':       // data dir
                db_dir_path = optarg;
                if(access(db_dir_path.c_str(), R_OK) != 0) {
                    std::cout << argv[0] << ": error: invalid db dir" << std::endl;
                    return 1;
                }
                break;
            case 'l':       // listen address
                listen_addr = optarg;
                break;
            case 't':       // number of threads
                num_threads = atoi(optarg);
                break;
            case ':':
                std::cout << argv[0] << ": error: option needs value" << std::endl\
                          << argv[0] << ": use -h for help" << std::endl;
                return 1;
            case '?':
                std::cout << argv[0] << ": error: unknown option: " << optopt << std::endl \
                          << argv[0] << ": use -h for help" << std::endl;
                return 1;
        }
    }

    
    threads.resize(num_threads);
    for (int i = 0; i < num_threads; i++)
        threads.at(i) = std::thread(RunServer, i);

    // wait for threads to finish
    for (auto& t : threads)
        t.join();
    return 0;
}
