/** @file
 * @brief
 *
 * @author
 *
 * @copyright 2020 The Chamberlain Group, Inc. All rights reserved. All
 *            information within this file and associated files, including all
 *            information and files transferred with this file are CONFIDENTIAL
 *            and the proprietary property of The Chamberlain Group, Inc.
 */

// ////////////////////////// Module Header File /////////////////////////////
#include "HalUartPosix.hpp"

// ///////////////////////// Package Header Files ////////////////////////////

// ////////////////////// Package Group Header Files /////////////////////////
// /////////////////// CGI Organizational Header Files ///////////////////////
// /////////////////// 3rd Party Software Header Files ///////////////////////
#include <fcntl.h>
#include <signal.h>
#include <cerrno>
#include <termios.h>
#include <unistd.h>
#include <memory.h>
#include <thread>
// ////////////////////// Standard C++ Header Files //////////////////////////
#include <string>
#include <iostream>
// /////////////////////// Standard C Header Files ///////////////////////////
#include <cstdio>
#include <cstdint>
#include <stdexcept>




namespace Hal
{

static struct termios tty;
static int tty_fd;

static const unsigned int max_read_bytes = 255;
static Uart::uart_buff_list_t * buf_list;

Uart::Uart(const uint32_t port,
           const uint32_t baud,
           const uint8_t data_bits,
           const uint8_t stop_bits,
           const uint8_t parity,
           const bool flow_ctrl)
: m_port(port),
m_rcv_flag(false)
{
    tty_fd = open(("/dev/ttyUSB" + std::to_string(port)).c_str(), O_RDWR | O_NOCTTY | O_SYNC);
    memset(&tty, 0, sizeof(tty));

    if(!tty_fd)
    {

        throw std::runtime_error("ERROR_OPEN_SERIAL");
    }

    if(tcgetattr(tty_fd, &tty) != 0)
    {
        close(tty_fd);
        throw std::runtime_error("ERROR_TCGETATTR");
    }


    // OUTPUT CONFIGURATION
    switch(data_bits)
    {
        case 5:
        {
            tty.c_cflag |= CS5;
            break;
        }

        case 6:
        {
            tty.c_cflag |= CS6;
            break;
        }

        case 7:
        {
            tty.c_cflag |= CS7;
            break;
        }

        case 8:
        {
            tty.c_cflag |= CS8;
            break;
        }

        default:
        {
            // Chunk size of 9 not handled by POSIX
            close(tty_fd);
            throw std::runtime_error("BAD_DATABITS");
        }
    }

    switch(stop_bits)
    {
        case 1:
        {
            tty.c_cflag &= ~(static_cast<uint32_t>(CSTOPB));
            break;
        }

        case 2:
        {
            tty.c_cflag |= CSTOPB;
            break;
        }

        default:
        {
            close(tty_fd);
            throw std::runtime_error("BAD_STOPBITS");
        }
    }

    switch (parity)
    {
        case 0:
        {
            tty.c_cflag &= ~(static_cast<uint32_t>(PARENB));
            break;
        }

        case 1:
        {
            tty.c_cflag |= (PARENB | PARODD);
            break;
        }

        case 2:
        {
            tty.c_cflag |= PARENB;
            break;
        }

        default:
        {
            close(tty_fd);
            throw std::runtime_error("BAD_PARITY");
        }
    }

    if(flow_ctrl)
    {
        tty.c_cflag |= CRTSCTS;
    }
    else
    {
        tty.c_cflag &= ~CRTSCTS;
    }

    // INPUT CONFIGURATION
    tty.c_lflag |= (static_cast<uint32_t>(ICANON));
    tty.c_lflag &= ~(static_cast<uint32_t>(ECHO));              // Disable echo
    tty.c_lflag &= ~(static_cast<uint32_t>(ECHOE));             // Disable erasure
    tty.c_lflag &= ~(static_cast<uint32_t>(ECHONL));            // Disable new-line echo
    tty.c_lflag &= ~(static_cast<uint32_t>(ISIG));              // Disable interpretation of INTR, QUIT and SUSP
    tty.c_iflag &= ~(static_cast<uint32_t>((IXON | IXOFF | IXANY)));        // Turn off s/w flow ctrl
    tty.c_iflag &= ~(static_cast<uint32_t>((IGNBRK|PARMRK|BRKINT|ISTRIP|INLCR|IGNCR|ICRNL)));           // Disable any special handling of received bytes
    tty.c_cc[VEOF] = '\0';

    tty.c_oflag &= ~(static_cast<uint32_t>(OPOST)); // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~(static_cast<uint32_t>(ONLCR)); // Prevent conversion of newline to carriage return/line feed

    // SPEED FOR I/O
    cfsetspeed(&tty, baud);

    if (tcsetattr(tty_fd, TCSANOW, &tty) != 0)
    {
        close(tty_fd);

        throw std::runtime_error("ERROR_SERIAL_ATTR");
    }
}

Uart::~Uart(void)
{
    close(tty_fd);
}

std::size_t Uart::send(std::uint8_t* data, std::size_t size, bool)
{
    return static_cast<std::size_t>(write(tty_fd, data, size));
}

std::size_t Uart::receive(std::uint8_t* data, std::size_t size)
{
    (void) data;
    (void) size;
    std::size_t ret = 0;

    if(m_rcv_buffers != nullptr)
    {
        m_rcv_flag = true;
        std::thread t([&](){
            while(m_rcv_flag == true)
            {
                if(m_rcv_buffers->size() ==  m_rcv_buffers->capacity())
                {
                    m_rcv_buffers->pop_front();
                }
                read(tty_fd, m_rcv_buffers->push_back(), max_read_bytes);
            }
        });
        t.detach();
    }
    return ret;
}

void Uart::set_rcv_buf(uart_buff_list_t * buffers)
{
    if(buffers != nullptr)
    {
        m_rcv_buffers = buffers;
        buf_list = buffers;
    }
}

Uart::RxStatus Uart::get_rx_status()
{
    RxStatus status = RxStatus::IDLE;
    std::size_t frames_ready = m_rcv_buffers->size() - 1;

    if(frames_ready > 0)
    {
        status = RxStatus::READY;
    }
    else
    {
        status = RxStatus::IDLE;
    }

    return status;
}


} // namesapece Hal
