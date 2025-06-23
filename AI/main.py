#!/usr/bin/env python3

from optparse import OptionParser
from AI.network_client import NetworkClient

def main():
    usage = "USAGE: %prog [options] arg\n -p\t\tthe portnumber\n -n\t\tthe name of the team\n -H\t\tthe name of the machine"
    parser = OptionParser(usage)
    parser.add_option("-p", "--port", dest="portnumber", help="is the port number")
    parser.add_option("-n", "--name", dest="name", help="is the name of the team")
    parser.add_option("-H", "--hostname", dest="hostname", help="is the name of the machine", default="localhost")
    parser.add_option("-i", "--id", dest="bot_id", help="is the client id", default="1")
    (options, _) = parser.parse_args()
    if not options.hostname or not options.portnumber or not options.name:
        parser.error("incorrect number of arguments")
        exit(1)
    hostname = options.hostname
    port = options.portnumber
    name = options.name
    bot_id = options.bot_id
    return (hostname, port, name, bot_id)

if __name__ == "__main__":
    hostname, port, name, bot_id = main()
    client = NetworkClient(hostname, port, name, bot_id)
    client.establish_connection()
    client.run_client()
    client.check_server_capacity()
    client.disconnect()
