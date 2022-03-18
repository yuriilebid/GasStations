#include "Connections/TcpConnection.h"
#include <curl/curl.h>
#include <netdb.h>
#include <unistd.h>
#include <iostream>
#include "logger.h"
#include <thread>
#include <fcntl.h>
#include <arpa/inet.h>
#include  <sys/types.h>
#include  <sys/socket.h>
#include  <sys/ioctl.h>
#include <errno.h>

using namespace std::chrono_literals;

TcpConnection::TcpConnection(const nlohmann::json& cfgObj) {
    try {
        address = cfgObj.at("addr");
        port = cfgObj.at("port");
        setTraceLogsState(cfgObj.at("trace"));
        setUniqueTraces(cfgObj.at("unique_logs"));

    } catch(const nlohmann::json::exception& e) {
        LogPrintf(spdlog::level::err, "TcpConnection error: {}", e.what());
    }
}

void TcpConnection::closeConnection() {
    close(fd);
}

bool TcpConnection::connectInit() {
    struct sockaddr_in serv_addr{};
    struct hostent *server;
    struct timeval tv{};

    tv.tv_sec = 1;
    tv.tv_usec = 0;

    fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(fd == -1) {
        LogPrintf(spdlog::level::err, "|TCP| Socket: create error {}", strerror(errno));
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        close(fd);
    }

    server = gethostbyname(address.c_str());
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *) server->h_addr,
          (char *) &serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(port);
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(struct timeval));
    LogPrintf(spdlog::level::info, "|TCP| connecting to |{}|", address.c_str());

    int connectStatus = connect(fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0;

    if(connectStatus == 0 and fd != 0) {
        LogPrintf(spdlog::level::info, "|TCP| connection success. FD: |{}| addr: |{}| port: |{}|", fd, address.c_str(), port);
        setConnectionInited(true);
        clearFailedReading();
        return true;
    } else {
        LogPrintf(spdlog::level::warn, "Connection to |{}:{}| failed: {}", address.c_str(), port, strerror(errno));
        setConnectionInited(false);
        close(fd);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        return false;
    }
}

std::chrono::steady_clock::time_point getTimeNowTcp() {
    return std::chrono::steady_clock::now();
}

std::vector<unsigned char>
TcpConnection::transmitByStartEnd(const std::vector<unsigned char> &packet, const std::vector<unsigned char> &start,
                                  const std::vector<unsigned char> &end, int sizeIndex, bool logTrace,
                                  int additionalSize) {
    std::vector<unsigned char> tracePacket;
    try {
        std::vector<uint8_t> responsePacket;
        uint8_t writeBuff[2048];
        uint8_t readBuff{};
        auto packetSize = packet.size();
        bool readCondition = true;

        for(int i = 0; i < packetSize; i++) {
            writeBuff[i] = packet[i];
        }

        auto timeNow = getTimeNowTcp();
        int startIn = 0;
        int startInEnd = start.size() - 1;
        int packetLen = 200;
        while(readCondition) {
            auto lastState = getTimeNowTcp();
            auto timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(timeNow - lastState).count();
            if(timeElapsed > 200) {
                LogPrintf(spdlog::level::warn, "|TCP| Max time waiting for complete packet");
                incrementFailedReading();
                LogTrace(spdlog::level::trace, "|TCP||READ| {:X}", fmt::join(tracePacket, " "));
                return responsePacket;
            }

            int nBytes = read(fd, &readBuff, 1);

            if (nBytes == 1) {
                tracePacket.push_back(readBuff);
                if (startIn <= startInEnd) {
                    if (readBuff == start.at(startIn)) {
                        startIn++;
                        responsePacket.push_back(readBuff);
                    } else if (startIn > 0) {
                        startIn = 0;
                        responsePacket.clear();
                    }
                } else {
                    if (startIn > startInEnd and (responsePacket.size() == sizeIndex - 1)) {
                        packetLen = readBuff;
                        printf("Found length of the packet: %X\n", readBuff);
                        responsePacket.push_back(readBuff);
                    } else {
                        if (responsePacket.size() != (packetLen + additionalSize)) {
                            responsePacket.push_back(readBuff);
                        }
                        if (responsePacket.size() >= (packetLen + additionalSize)) {
                            if (getTraceLogsState() and logTrace and !getUniqueTraces()) {
                                LogConsole(spdlog::level::trace, "|TCP||READ| {:X}",
                                          fmt::join(responsePacket, " "));
                            }
                            LogFile(spdlog::level::trace, "|TCP||READ| {:X}",
                                          fmt::join(responsePacket, " "));
                            LogTrace(spdlog::level::trace, "|TCP||READ| {:X}", fmt::join(tracePacket, " "));
                            return responsePacket;
                        }
                    }
                }
            }
        }
        return responsePacket;
    } catch(std::exception& e) {
        LogFile(spdlog::level::warn, "Got std error: {}", e.what());
        LogTrace(spdlog::level::trace, "|TCP||READ| {:X}", fmt::join(tracePacket, " "));
        return {};
    } catch(std::out_of_range& e) {
        LogFile(spdlog::level::warn, "Got out of range packet: {}", e.what());
        LogTrace(spdlog::level::trace, "|TCP||READ| {:X}", fmt::join(tracePacket, " "));
        return {};
    }
}

std::vector<unsigned char> TcpConnection::transmit(const std::vector<unsigned char> &packet, bool logTrace) {
    std::vector<uint8_t> responsePacket;
    uint8_t writeBuff[2048];
    uint8_t readBuff[512];
    auto packetSize = packet.size();

    bzero(readBuff, 512);
    for(int i = 0; i < packetSize; i++) {
        writeBuff[i] = packet[i];
    }
    LogTrace(spdlog::level::trace, "|TCP|{}|{}|WRITE| {:X}", fd, address, fmt::join(packet, " "));
    if(getTraceLogsState() and logTrace and !getUniqueTraces()) {
        LogPrintf(spdlog::level::trace, "|TCP|{}|{}|WRITE| {:X}", fd, address, fmt::join(packet, " "));
    }
    lastWrote = packet;
    int writeBytes = send(fd, &writeBuff, packetSize, 0);
    std::this_thread::sleep_for(10ms);
    int nBytes = read(fd, &readBuff, 512);

    for(int i = 0; i < nBytes; i++) {
        responsePacket.push_back(readBuff[i]);
    }

    LogTrace(spdlog::level::trace, "|TCP|{}|{}|READ| {:X}", nBytes, address, fmt::join(responsePacket, " "));
    if (getTraceLogsState() and logTrace and !getUniqueTraces()) {
        LogPrintf(spdlog::level::trace, "|TCP||{}||{}||READ| {:X}", nBytes, address,
                  fmt::join(responsePacket, " "));
    }
    if (nBytes < 1) {
        incrementFailedReading();
    }
    return responsePacket;
}

void TcpConnection::writePacket(const std::vector<unsigned char> &packet, bool logTrace = false) {
    write(fd, packet.data(), packet.size());

    LogTrace(spdlog::level::trace, "|TCP||{}||WRITE| {:X}", address, fmt::join(packet, " "));
    if(getTraceLogsState() and !(getUniqueTraces() and packet == lastWrote)) {
        LogPrintf(spdlog::level::trace, "|TCP||{}||WRITE| {:X}", address, fmt::join(packet, " "));
    }
    lastWrote = packet;
}

std::vector<unsigned char> TcpConnection::readPacketBySize(bool logTrace, int sizeIndex) {
    char readByte;
    std::vector<unsigned char> responsePacket;

    while (sizeIndex >= 0) {
        int nBytes = read(fd, &readByte, 1);
        if (nBytes == 1) {
            responsePacket.push_back(readByte);
            sizeIndex--;
        }
    }
    int restSize = readByte - sizeIndex;
    while (restSize > 0) {
        int nBytes = read(fd, &readByte, 1);
        if (nBytes == 1) {
            responsePacket.push_back(readByte);
            sizeIndex--;
        }
    }
    LogTrace(spdlog::level::trace, "|TCP_EXTENDED||{}||{}||READ| {:X}", restSize, address,
             fmt::join(responsePacket, " "));
    LogPrintf(spdlog::level::trace, "|TCP_EXTENDED||{}||{}||READ| {:X}", restSize, address,
              fmt::join(responsePacket, " "));
    return responsePacket;
}

std::vector<unsigned char> TcpConnection::readPacketByByte(bool logTrace, int expectedSize) {
    char readByte;
    std::vector<unsigned char> responsePacket;

    while(expectedSize > 0) {
        int nBytes = read(fd, &readByte, 1);

        if(nBytes == 1) {
            responsePacket.push_back(readByte);
            expectedSize--;
        }
    }
    LogTrace(spdlog::level::trace, "|TCP_EXTENDED||{}||{}||READ| {:X}", expectedSize, address, fmt::join(responsePacket, " "));
    LogPrintf(spdlog::level::trace, "|TCP_EXTENDED||{}||{}||READ| {:X}", expectedSize, address, fmt::join(responsePacket, " "));
    return responsePacket;
}

std::vector<unsigned char> TcpConnection::readPacket(bool logTrace) {
    std::vector<unsigned char> responsePacket;
    uint8_t readBuff[512];

    bzero(readBuff, 512);
    int nBytes = read(fd, &readBuff, 512);

    for(int i = 0; i < nBytes; i++) {
        responsePacket.push_back(readBuff[i]);
    }
    LogTrace(spdlog::level::trace, "|TCP||{}||{}||READ| {:X}", nBytes, address, fmt::join(responsePacket, " "));
    if(getTraceLogsState() and !(getUniqueTraces() and responsePacket == lastPacket) and logTrace) {
        LogPrintf(spdlog::level::trace, "|TCP||{}||{}||READ| {:X}", nBytes, address, fmt::join(responsePacket, " "));
    }

    if(nBytes < 1) {
       incrementFailedReading();
    }
    lastPacket = responsePacket;
    return responsePacket;
}