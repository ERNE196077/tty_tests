#ifndef HAL_UART_HPP
#define HAL_UART_HPP

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

#include "BufList.hpp"
#include <atomic>

namespace Hal
{

class Uart
{
public:
    enum class RxStatus
    {
        READY,
        BUSY,
        IDLE,
        ERROR
    };
    
    typedef Hal::BufList<1024, 3> uart_buff_list_t;

    Uart(const uint32_t port,
         const uint32_t baud,
         const uint8_t data_bits = 8,
         const uint8_t stop_bits = 1,
         const uint8_t parity = 0,
         const bool flow_ctrl = false);
    ~Uart();

    std::size_t send(std::uint8_t* data, std::size_t size, bool blocking);
    std::size_t receive(std::uint8_t* data, std::size_t size);
    RxStatus get_rx_status();
    void set_rcv_buf(uart_buff_list_t * buffers);

private:
    uart_buff_list_t * m_rcv_buffers;
    std::uint32_t m_port;
    std::atomic<bool> m_rcv_flag;
};

} // nanmespace hal

#endif // HAL_UART_HPP
