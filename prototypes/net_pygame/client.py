import sys
import threading
import socket

import pygame
from pygame.locals import *


print_packets = 0


class Vec:
    x = 0
    y = 0

    def __init__(self, X = 0, Y = 0):
        self.x = X
        self.y = Y

    def as_tuple(self):
        return (self.x, self.y)

    def add(self, v):
        return Vec(self.x + v.x, self.y + v.y)
        
    def mul(self, v):
        return Vec(self.x * v.x, self.y * v.y)


class Entity:
    # image
    # id
    # pos
    # vel

    def __init__(self, image, id, pos):
        self.image = image
        self.id = id
        self.pos = pos
        self.vel = Vec(0, 0)

    def update(self, dt):
        self.pos.x += self.vel.x * dt
        self.pos.y += self.vel.y * dt


class NetworkThread(threading.Thread):
    def __init__(self, listener):
        self.listener = listener
        
        threading.Thread.__init__(self)
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        host = 'localhost'
        if len(sys.argv) >= 2:
            host = sys.argv[1]
        port = 8765

        print "Connecting to %s, port %d" % (host, port)
        self.socket.connect((host, port))

    def run(self):
        print "Connected to server"
        
        data = ''
        while 1:
            try:
                read = self.socket.recv(1024)
                if len(read) == 0:
                    break
            except:
                break

            data += read.replace('\r', '')

            eol_index = data.find('\n')
            if eol_index != -1:
                line = data[0:eol_index]
                self.process_packet(line)
                data = data[eol_index+1:]

        print "Disconnected from server"
        self.socket.close()

    def process_packet(self, line):
        if print_packets:
            print "Processing packet: " + line
        self.listener.recieve(line)

    def send(self, packet):
        if print_packets:
            print "Sending: " + repr(packet)
        try:
            self.socket.sendall(packet + '\n')
        except:
            pass


class ServerConnection:
    queue = []
    lock = threading.RLock()
    # net_thread = NetworkThread()

    def __init__(self):
        self.net_thread = NetworkThread(self)
        self.net_thread.start()
        self.net_thread.send('login')

    def close(self):
        if self.net_thread:
            self.net_thread.send('logout')

    def recieve(self, packet):
        self.lock.acquire()
        self.queue.append(packet)
        self.lock.release()

    def send(self, packet):
        self.net_thread.send(packet)

    def get_packets(self):
        self.lock.acquire()
        queue = self.queue
        self.queue = []
        self.lock.release()
        return queue


class Client:
    # player
    # shot
    # connection
    
    # id          <-- id of self
    others = {} # <-- other entities: id -> Entity

    def main(self):
        pygame.init()
        self.screen = pygame.display.set_mode((400, 400))
        pygame.display.set_caption('Empyrean Net Test')

        clock = pygame.time.Clock()

        self.player = pygame.image.load("player.png")
        self.shot   = pygame.image.load("shot.png")
        self.connection = ServerConnection()

        while 1:
            dt = float(clock.tick(60)) / 1000

            if self.process_events():
                break
            if self.process_packets():
                break

            self.update(dt)
            self.draw()
            pygame.display.flip()

        self.connection.close()

    def process_events(self):
        for event in pygame.event.get():
            if event.type == QUIT:
                return 1
            elif event.type == KEYDOWN:
                if event.key == K_ESCAPE:
                    return 1
                elif event.key == K_UP:
                    self.connection.send('addvel 0 -1')
                elif event.key == K_RIGHT:
                    self.connection.send('addvel 1 0')
                elif event.key == K_DOWN:
                    self.connection.send('addvel 0 1')
                elif event.key == K_LEFT:
                    self.connection.send('addvel -1 0')
                elif event.key == K_SPACE:
                    self.connection.send('fire')
            elif event.type == KEYUP:
                if event.key == K_UP:
                    self.connection.send('addvel 0 1')
                elif event.key == K_RIGHT:
                    self.connection.send('addvel -1 0')
                elif event.key == K_DOWN:
                    self.connection.send('addvel 0 -1')
                elif event.key == K_LEFT:
                    self.connection.send('addvel 1 0')


    def process_packets(self):
        packets = self.connection.get_packets()
        for p in packets:
            self.process_packet(p)

    def process_packet(self, packet):
        ops = packet.split()
        if len(ops):
            command = ops[0]
            args = ops[1:]
            if command == "id":
                self.id = int(args[0])
            elif command == "update":
                id = int(args[0])
                x = float(args[1])
                y = float(args[2])
                vx = float(args[3])
                vy = float(args[4])
                self.others[id].pos = Vec(x, y)
                self.others[id].vel = Vec(vx, vy)
            elif command == "added":
                image = str(args[0])
                id = int(args[1])
                x = float(args[2])
                y = float(args[3])
                self.others[id] = Entity(image, id, Vec(x, y))
            elif command == "removed":
                id = int(args[0])
                del self.others[id]

    def update(self, dt):
        for o in self.others.values():
            o.update(dt)

    def draw(self):
        self.screen.fill(0)
        for o in self.others.values():
            image = (o.image == "player.png" and self.player or self.shot)
            offset = Vec(-image.get_width() / 2, -image.get_height() / 2)
            pos = o.pos.add(Vec(2, 2)).mul(Vec(100, 100)).add(offset).as_tuple()
            if o.image == "player.png":
                self.screen.blit(image, pos)
            else:
                self.screen.blit(self.shot, pos)

if __name__ == '__main__':
    Client().main()
