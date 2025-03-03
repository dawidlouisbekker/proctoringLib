import socket
import threading
import random
from typing import Literal
HOST = "127.0.0.1"
PORT = 8080

def genBind(client : socket.socket):
    valid : bool = False
    sock : socket.socket = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
    port : int = 0
    while valid == False:
        try:
            port = random.randint(10,64000)
            sock.bind(HOST,port)
            valid = True
        except:
            continue
    asc_port = str(port).encode('ascii')
    client.send(asc_port)
    return sock
            
def startServer(sock : socket.socket, option : str):

    sock.listen()
    print("Server thread started listening")
    conn, addr = sock.accept()
    match option:
        case "audio":
            
            pass
        case "video":
            pass
        case "screenshot":
            pass
    

    

def startProcSesh(client : socket.socket):
    pass

#might connect to eduvos server or 3rd party incase of session cancelation.
if __name__ == "__main__":
    sock = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
    sock.bind((HOST,PORT))
    sock.listen()
    print("Main server thread started listening")
    client, addr = sock.accept()
    screenShotSock = genBind(client=client)
    vidSock = genBind(client=client)
    try:
        t1 = threading.Thread(target=startServer,args=(screenShotSock,"screenshot",),daemon=True)
        t2 = threading.Thread(target=startServer,args=(vidSock,"video",),daemon=True)
        t1.start()
        t2.start()
    except:
        screenShotSock.close()
        vidSock.close()
        t1.join()
        t2.join()
    

