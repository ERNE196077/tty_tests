#include <iostream>
#include <unistd.h>
#include <signal.h>

#include "HalUartPosix.hpp"

using namespace std;


int main()
{
    std::uint32_t  timeout = 20;
    Hal::Uart::uart_buff_list_t mylist;
    Hal::Uart uart(6, 9600);
    uart.set_rcv_buf(&mylist);
    std::cout << "STARTING: " << std::endl << std::flush;
    uart.receive(nullptr, 0);
    while(timeout > 1)
    {
        if(uart.get_rx_status() != Hal::Uart::RxStatus::READY)
        {
            std::cout << "NEW MESSAGE: " << mylist.back() << std::endl << std::flush;
            mylist.pop_back();
        }
        sleep(1);
        timeout--;
    }
    std::cout << "FINISHING: " << std::endl << std::flush;
    return 0;
}
