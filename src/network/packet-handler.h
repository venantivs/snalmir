#ifndef __PACKET_HANDLER_H__
#define __PACKET_HANDLER_H__

bool	segregate_packet(unsigned char *packet, int user_index);
void  print_message(unsigned char *packet);

#endif
