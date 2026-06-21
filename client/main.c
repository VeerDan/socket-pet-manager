#include "client.h"
#include "logic.h"
#include "tui.h"
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    return_code_t rc = ERR_OK;
    return_code_t cleanup_rc = ERR_OK;
    int sock = -1;

    rc = connect_to_server(SERVER_IP, SERVER_PORT, &sock);
    if (rc != ERR_OK)
        print_error_message(rc);

    if (rc == ERR_OK)
    {
        rc = start_client_ui(sock);
        if (rc != ERR_OK)
            print_error_message(rc);
    }

    if (sock >= 0)
    {
        cleanup_rc = close_connection(sock);
        if (cleanup_rc != ERR_OK)
            print_error_message(cleanup_rc);
    }

    return rc;
}
