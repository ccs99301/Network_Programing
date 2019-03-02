#include <stdio.h>
#include <string.h>
#include <pcap.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef struct eth_hdr
{
	u_char dst_mac[6];
	u_char src_mac[6];
	u_short eth_type;
}eth_hdr;

eth_hdr *ethernet;

typedef struct ip_hdr
{
	int version:4;
	int header_len:4;
	u_char tos:8;
	int total_len:16;
	int ident:16;
	int flags:16;
	u_char ttl:8;
	u_char protocol:8;
	int checksum:16;
	u_char sourceIP[4];
	u_char destIP[4];
}ip_hdr;

ip_hdr *ip;

typedef struct ip6_hdr
{
	unsigned int version:4;
	unsigned int traffic_class:8;
	unsigned int flow_label:20;
	uint16_t payload_len;
	uint8_t next_header;
	uint8_t hop_limit;
	uint16_t sourceIP[8];
	uint16_t destIP[8];
}ip6_hdr;

ip6_hdr *ip6;

typedef struct tcp_hdr
{
	u_short sport;
	u_short dport;
	u_int seq;
	u_int ack;
	u_char head_len;
	u_char flags;
	u_short wind_size;
	u_short check_sum;
	u_short urg_ptr;
}tcp_hdr;

tcp_hdr *tcp;

typedef struct udp_hdr
{
	u_short sport;
	u_short dport;
	u_short tot_len;
	u_short check_sum;
}udp_hdr;

udp_hdr *udp;

void packet_handler(u_char *user, const struct pcap_pkthdr *pkt_header, const u_char *pkt_data)
{
	static int count=0;
	count++;
	printf("packet #%d\n",count);
	printf("capture time : %s",ctime((const time_t*)&pkt_header->ts.tv_sec));
	printf("packet length : %d\n", pkt_header->len);
	u_int eth_len=sizeof(struct eth_hdr);
	u_int ip_len=sizeof(struct ip_hdr);
	u_int ip6_len=sizeof(struct ip6_hdr);
	u_int tcp_len=sizeof(struct tcp_hdr);
	u_int udp_len=sizeof(struct udp_hdr);
	ethernet=(eth_hdr *)pkt_data;
	if(ntohs(ethernet->eth_type)==0x0800)
	{
        	printf("IPv4 is used\n");
		printf("IPv4 header information:\n");
		ip=(ip_hdr*)(pkt_data+eth_len);
		printf("Source IP : %d.%d.%d.%d\n",ip->sourceIP[0],ip->sourceIP[1],ip->sourceIP[2],ip->sourceIP[3]);
		printf("Dest IP : %d.%d.%d.%d\n",ip->destIP[0],ip->destIP[1],ip->destIP[2],ip->destIP[3]);
		if(ip->protocol==6)
		{
            		printf("TCP is used:\n");
			tcp=(tcp_hdr*)(pkt_data+eth_len+ip_len);
			printf("TCP source port : %u\n",htons(tcp->sport));
			printf("TCP dest port : %u\n",htons(tcp->dport));
		}
		else if(ip->protocol==17)
		{
			printf("UDP is used:\n");
			udp=(udp_hdr*)(pkt_data+eth_len+ip_len);
			printf("UDP source port : %u\n",htons(udp->sport));
			printf("UDP dest port : %u\n",htons(udp->dport));
        	}
		else
		{
			printf("other transport protocol is used\n");
		}
	}
	else if(ntohs(ethernet->eth_type)==0x086dd)
	{
		printf("IPv6 is used\n");
		ip6=(ip6_hdr*)(pkt_data+eth_len);
		char str[INET6_ADDRSTRLEN];
		printf("Source IPv6 : %s\n",inet_ntop(AF_INET6,ip6->sourceIP,str,sizeof(str)));
		printf("Dest IPv6 : %s\n",inet_ntop(AF_INET6,ip6->destIP,str,sizeof(str)));
                if(ip6->next_header==6)
                {
                        printf("TCP is used:\n");
                        tcp=(tcp_hdr*)(pkt_data+eth_len+ip6_len);
                        printf("TCP source port : %u\n",htons(tcp->sport));
                        printf("TCP dest port : %u\n",htons(tcp->dport));
                }
                else if(ip6->next_header==17)
                {
                        printf("UDP is used:\n");
                        udp=(udp_hdr*)(pkt_data+eth_len+ip6_len);
                        printf("UDP source port : %u\n",htons(udp->sport));
                        printf("UDP dest port : %u\n",htons(udp->dport));
                }
                else
                {
                        printf("Other transport protocol is used\n");
                }
	}
	else
    	{
		printf("Other ethernet_type\n");
	}
	printf("----------------------------------------\n");
}

int main(int argc,char *argv[])
{
    	pcap_t *handle;
    	char errbuf[PCAP_ERRBUF_SIZE];
    	bpf_u_int32 mask;
    	bpf_u_int32 net;
    	struct bpf_program filter;	
       	handle=pcap_open_offline(argv[1],errbuf);
	if(argc>1)
	{
    		pcap_compile(handle,&filter,argv[2],0,net);
    		pcap_setfilter(handle,&filter);
	}
    	pcap_loop(handle,-1,packet_handler,NULL);
    	pcap_close(handle);
    	return(0);
}
