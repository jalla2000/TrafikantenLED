
#include <avrstuff>

byte displayBuffer[32*128*2];

void interrupt_for_every_received_byte(byte)
{
    put_in_displaybuffer(byte);
}

int main()
{
    setup_serial_port();
    setup_interrupts();

    while(1) {
        for every_every_byte_in_displaybuffer {
            draw_bytes_on_display();
        }
    }
}
