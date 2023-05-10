#include "capture.h"
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

int init_capture(struct capture_t *capture, struct pcap_header_t *header)
{
    capture->header = header;
    capture->index = 0;
    capture->size = 1;
    capture->packets = malloc(sizeof(struct packet_t));
    if (capture->packets == NULL) {
        return -1;
    }
    return 0;
}

int realloc_packets(struct packet_t **packets, size_t new_size)
{
    struct packet_t *new_loc = (struct packet_t *) realloc(*packets, new_size);
    if (new_loc == NULL) {
        return -1;
    }
    *packets = new_loc;
    return 0;
}

int load_capture(struct capture_t *capture, const char *filename)
{
    if (capture == NULL) {
        return -1;
    }
    if (filename == NULL) {
        return -1;
    }
    struct pcap_context context[1];
    if (init_context(context, filename) != 0) {
        return -1;
    }
    struct pcap_header_t *pcap_header = malloc(sizeof(struct pcap_header_t));
    if (pcap_header == NULL) {
        destroy_context(context);
        return -1;
    }
    if ((load_header(context, pcap_header) != 0) ||
    (init_capture(capture, pcap_header) == -1)) {
        free(pcap_header);
        destroy_context(context);
        return -1;
    }
    bool success_load = true;
    while (success_load) {
        struct packet_t *packet = malloc(sizeof(struct packet_t));
        if (packet == NULL) {
            destroy_capture(capture);
            destroy_context(context);
            return -1;
        }
        if (load_packet(context, packet) != 0) {
            success_load = false;
            free(packet);
            continue;
        }
        if ((capture->index) + 1 >= (int) capture->size) {
            capture->size *= 2;
            if (realloc_packets(&capture->packets,
                                (capture->size) * sizeof(struct packet_t)) == -1) {
                free(packet);
                destroy_capture(capture);
                destroy_context(context);
                return -1;
            }
        }
        capture->packets[capture->index] = *packet;
        free(packet);
        capture->index += 1;
    }
    capture->index -= 1;
    destroy_context(context);
    return 0;
}

void destroy_capture(struct capture_t *capture)
{
    if (capture == NULL) {
        return;
    }
    free(capture->header);
    for (int i = 0; i <= capture->index; i++) {
        destroy_packet(&capture->packets[i]);
    }
    free(capture->packets);
}

const struct pcap_header_t *get_header(const struct capture_t *const capture)
{
    if (capture == NULL) {
        return NULL;
    }
    return capture->header;
}

struct packet_t *get_packet(
        const struct capture_t *const capture,
        size_t index)
{
    if ((int) index > capture->index) {
        return NULL;
    }
    return &capture->packets[index];
}

size_t packet_count(const struct capture_t *const capture)
{
    return capture->index + 1;
}

size_t data_transfered(const struct capture_t *const capture)
{
    size_t result = 0;
    for (int i = 0; i <= capture->index; i++) {
        result += capture->packets[i].packet_header->orig_len;
    }
    return result;
}

bool operation_mask(struct packet_t *packet, uint8_t s_ip[4], uint8_t d_ip[4], void *data)
{
    uint8_t *length = (uint8_t *) data;
    for (int i = 0; i < (int) *length / 8; i++) {
        bool result_s = true;
        bool result_d = true;
        if (s_ip != NULL) {
            result_s = packet->ip_header->src_addr[i] == s_ip[i];
        }
        if (d_ip != NULL) {
            result_d = packet->ip_header->dst_addr[i] == d_ip[i];
        }
        if (!(result_s && result_d)) {
            return false;
        }
    }
    return true;
}

bool operation_size(struct packet_t *packet, uint8_t s_ip[4], uint8_t d_ip[4], void *data)
{
    (void) s_ip;
    (void) d_ip;
    uint32_t *size = (uint32_t *) data;
    return packet->packet_header->orig_len >= *size;
}

bool operation_protocol(struct packet_t *packet, uint8_t s_ip[4], uint8_t d_ip[4], void *data)
{
    (void) s_ip;
    (void) d_ip;
    uint8_t *protocol = (uint8_t *) data;
    return packet->ip_header->protocol == *protocol;
}

int filter_packets(
        const struct capture_t *const original,
        struct capture_t *filtered,
        bool (*operation) (struct packet_t*, uint8_t[4], uint8_t[4], void*),
        uint8_t s_ip[4], uint8_t d_ip[4], void *data)
{
    struct pcap_header_t *filter_header = (struct pcap_header_t*) malloc(sizeof(struct pcap_header_t));
    if (filter_header == NULL) {
        return -1;
    }
    memcpy(filter_header, original->header, sizeof(struct pcap_header_t));

    if (init_capture(filtered, filter_header) == -1) {
        free(filter_header);
        return -1;
    }
    int pck_count = (int) packet_count(original);
    for (int i = 0; i < pck_count; i++) {

        struct packet_t *org_packet = get_packet(original, i);
        if (data != NULL) {
            if (!operation(org_packet, s_ip, d_ip, data)) {
                continue;
            }
        }

        if ((filtered->index) + 1 >= (int) filtered->size) {
            filtered->size *= 2;
            if (realloc_packets(&filtered->packets,
                                filtered->size * sizeof(struct packet_t)) == -1) {
                destroy_capture(filtered);
                return -1;
            }
        }
        struct packet_t *filtered_packet = (struct packet_t*) malloc(sizeof(struct packet_t));
        if (filtered_packet == NULL) {
            destroy_capture(filtered);
            return -1;
        }
        int c = copy_packet( org_packet, filtered_packet);
        if (c != 0) {
            free(filtered_packet);
            destroy_capture(filtered);
            return -1;
        }
        filtered->packets[filtered->index] = *filtered_packet;
        filtered->index += 1;
        free(filtered_packet);
    }
    filtered->index -= 1;
    return 0;
}

int filter_protocol(
        const struct capture_t *const original,
        struct capture_t *filtered,
        uint8_t protocol)
{
    if (original == NULL || filtered == NULL) {
        return -1;
    }

    return filter_packets(
            original, filtered, operation_protocol, NULL, NULL, &protocol);
}

int filter_larger_than(
        const struct capture_t *const original,
        struct capture_t *filtered,
        uint32_t size)
{
    if (original == NULL || filtered == NULL) {
        return -1;
    }
    return filter_packets(
            original, filtered, operation_size, NULL, NULL, &size);
}

int filter_from_to(
        const struct capture_t *const original,
        struct capture_t *filtered,
        uint8_t source_ip[4],
        uint8_t destination_ip[4])
{
    if (original == NULL || filtered == NULL) {
        return -1;
    }
    uint8_t mask_length = 32U;
    return filter_packets(
            original, filtered, operation_mask,
            source_ip, destination_ip, &mask_length);
}

int filter_from_mask(
        const struct capture_t *const original,
        struct capture_t *filtered,
        uint8_t network_prefix[4],
        uint8_t mask_length)
{
    if (original == NULL || filtered == NULL || mask_length > 32) {
        return -1;
    }
    return filter_packets(
            original, filtered, operation_mask,
            network_prefix, NULL, &mask_length);
}

int filter_to_mask(
        const struct capture_t *const original,
        struct capture_t *filtered,
        uint8_t network_prefix[4],
        uint8_t mask_length)
{
    if (original == NULL || filtered == NULL || mask_length > 32) {
        return -1;
    }
    return filter_packets(original, filtered, operation_mask, NULL,
                          network_prefix, &mask_length);
}

bool in_flow(uint8_t *s_ip, uint8_t *d_ip, struct capture_t *flows, size_t size)
{
    for (int i = 0; i < (int) size; i++) {
        struct capture_t flow = flows[i];
        uint8_t length = 32U;
        if (operation_mask(flow.packets, s_ip, d_ip, &length)) {
            return true;
        }
    }
    return false;
}

int get_flows(size_t array_size, const struct capture_t *const capture, struct capture_t flows[array_size][1], size_t *size)
{
    for (int i = 0; i <= capture->index; i++) {
        uint8_t *s_ip = capture->packets[i].ip_header->src_addr;
        uint8_t *d_ip = capture->packets[i].ip_header->dst_addr;
        if (in_flow(s_ip, d_ip, *flows, *size)) {
            continue;
        }
        if (filter_from_to(capture, flows[*size], s_ip, d_ip) != 0) {
            for (int j = 0; j < (int) *size; j++) {
                destroy_capture(flows[j]);
            }
            return -1;
        }
        *size += 1;
    }
    return 0;
}

int print_flow_stats(const struct capture_t *const capture)
{
    if (capture == NULL) {
        fprintf(stderr,"Capture is NULL.\n");
        return -1;
    }

    uint8_t c_size = packet_count(capture);
    struct capture_t flows[c_size][1];
    size_t size = 0;
    if (get_flows(c_size, capture, flows, &size) != 0) {
        fprintf(stderr, "Find flows error.\n");
        return -1;
    }
    for (int i = 0; i < (int) size; i++) {
        size_t count = packet_count(flows[i]);
        uint8_t *s_ip = flows[i]->packets[0].ip_header->src_addr;
        uint8_t *d_ip = flows[i]->packets[0].ip_header->dst_addr;
        printf("%u.%u.%u.%u -> %u.%u.%u.%u : %zu\n",
               s_ip[0], s_ip[1], s_ip[2], s_ip[3], d_ip[0], d_ip[1], d_ip[2], d_ip[3], count);
    }
    for (int i = 0; i < (int) size; i++) {
        destroy_capture(flows[i]);
    }
    return 0;
}

uint32_t count_sec(struct capture_t *flow) {
    struct packet_t fst_pcap = flow->packets[0];
    struct packet_t last_pcap = flow->packets[flow->index];
    return last_pcap.packet_header->ts_sec - fst_pcap.packet_header->ts_sec;
}

uint32_t count_n_sec(struct capture_t *flow) {
    struct packet_t fst_pcap = flow->packets[0];
    struct packet_t last_pcap = flow->packets[flow->index];
    uint32_t magic_num = flow->header->magic_number;
    // return time in micro
    uint32_t usec = last_pcap.packet_header->ts_usec - fst_pcap.packet_header->ts_usec;
    return (magic_num == 0xA1B23C4D) ? (usec * 1000) : usec;
}

void get_longest(size_t array_size, size_t size, struct capture_t flows[array_size][1], struct capture_t **longest)
{
    for (int i = 1; i < (int) size; i++) {
        uint32_t longest_sec = count_sec(*longest);
        uint32_t longest_n_sec = count_n_sec(*longest);
        uint32_t sec = count_sec(flows[i]);
        uint32_t n_sec = count_n_sec(flows[i]);
        if (sec > longest_sec) {
            *longest = flows[i];
        } else if ((sec == longest_sec) && (n_sec > longest_n_sec)) {
            *longest = flows[i];
        }
    }
}

int print_longest_flow(const struct capture_t *const capture)
{
    if (capture == NULL) {
        fprintf(stderr, "Capture is NULL.\n");
        return -1;
    }
    uint8_t c_size = packet_count(capture);
    if (c_size == 0) {
        fprintf(stderr, "Empty capture.\n");
        return -1;
    }
    struct capture_t flows[c_size][1];
    size_t size = 0;
    if (get_flows(c_size, capture, flows, &size) != 0) {
        fprintf(stderr, "Find flows error.\n");
        return -1;
    }
    struct capture_t *longest = flows[0];
    if ((int) size >= 2) {
        get_longest(c_size, size, flows, &longest);
    }
    uint8_t *s_ip = longest->packets[0].ip_header->src_addr;
    uint8_t *d_ip = longest->packets[0].ip_header->dst_addr;
    printf("%u.%u.%u.%u -> %u.%u.%u.%u : %d:%d - %d:%d\n",
           s_ip[0], s_ip[1], s_ip[2], s_ip[3], d_ip[0], d_ip[1], d_ip[2], d_ip[3],
           longest->packets[0].packet_header->ts_sec, longest->packets[0].packet_header->ts_usec,
           longest->packets[longest->index].packet_header->ts_sec,
           longest->packets[longest->index].packet_header->ts_usec);
    for (int i = 0; i < (int) size; i++) {
        destroy_capture(flows[i]);
    }
    return 0;
}
