#!/usr/bin/env python3
import os
import sys
import getopt
import grpc
from project1_pb2 import Query
from project1_pb2_grpc import ImgSearchEngine
from project1_pb2_grpc import ImgSearchEngineStub

def save_image(filename, bytes):
    with open(filename, "wb") as outfile:
        outfile.write(bytes)

class Client:
    stub = 0
    def __init__(self, channel):
        self.stub = ImgSearchEngineStub(channel)

    def make_query(self, query):
        request = Query(query=query)
        img = self.stub.MakeQuery(request)
        return img

def main(argv):
    server_host = "localhost:50051"
    query = ""
    outfile = ""

    # Get server host from environment variable
    if "SERVER_HOST" in os.environ:
        server_host = os.environ["SERVER_HOST"]

    try:
        opts, args = getopt.getopt(argv, "hs:q:o:")
    except getopt.GetoptError:
        print("Usage: " + sys.argv[0] + " [options] -o <outfile> -q <query>")
        print("   -h                   - display this help")
        print("   -s host:port         - specify server")
        print("   -o /path/to/img.jpg  - filename for image")
        print("   -q query             - keyword to search")
        sys.exit(2)

    for opt, arg in opts:
        if opt == "-h":
            print("Usage: " + sys.argv[0] + " [options] -o <outfile> -q <query>")
            print("   -h                   - display this help")
            print("   -s host:port         - specify server")
            print("   -o /path/to/img.jpg  - filename for image")
            print("   -q query             - keyword to search")
            sys.exit()
        elif opt == "-s":
            server_host = arg
        elif opt == "-q":
            query = arg
        elif opt == "-o":
            outfile = arg

    if not query or not outfile:
        print(sys.argv[0] + ": error: must specify query and outfile")
        sys.exit(1)

    # Create a gRPC channel
    channel = grpc.insecure_channel(server_host)

    # Create the gRPC client
    img_search_client = Client(channel)

    # Send request to the server
    img = img_search_client.make_query(query)

    if img.error == 200:
        save_image(outfile, img.image)
        print("Downloaded image successfully to '" + outfile + "'!")
    elif img.error == 404:
        print("No results for: '" + query + "'")
    else:
        print("Server error")

if __name__ == "__main__":
    main(sys.argv[1:])
