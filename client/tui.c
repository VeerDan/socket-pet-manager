#include "tui.h"
#include "client.h"
#include "logic.h"
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

static void init_ui_layout(int hist_start_y);
static void get_tui_input(int starty, int startx, const char *prompt, char *buffer, int max_len);
static void log_history(WINDOW *win, const char *cmd_name, const char *args, response_t *res);

static const char *choices[] = {"Add user",   "Delete user (and their pets)", "Add pet to user",
                                "Delete pet", "Show all users and pets",      "Exit"};

static void init_ui_layout(int hist_start_y)
{
    attron(A_BOLD);
    mvprintw(0, 0, "STATUS: Connected to server %s:%d", SERVER_IP, SERVER_PORT);
    attroff(A_BOLD);

    mvprintw(2, 0, "--- MENU (UP/DOWN arrows and ENTER) ---");

    attron(COLOR_PAIR(4));
    mvhline(hist_start_y - 1, 0, ACS_HLINE, COLS);
    mvprintw(hist_start_y - 1, 2, " ACTION HISTORY ");
    attroff(COLOR_PAIR(4));

    refresh();
}

static void get_tui_input(int starty, int startx, const char *prompt, char *buffer, int max_len)
{
    mvprintw(starty, startx, "%s", prompt);
    clrtoeol();
    echo();
    curs_set(1);

    getnstr(buffer, max_len - 1);

    noecho();
    curs_set(0);

    move(starty, 0);
    clrtoeol();
}

static void log_history(WINDOW *win, const char *cmd_name, const char *args, response_t *res)
{
    wattron(win, A_BOLD);
    wprintw(win, "> %s", cmd_name);
    wattroff(win, A_BOLD);

    if (args && strlen(args) > 0)
        wprintw(win, " [%s]", args);

    wprintw(win, "\n");

    if (res->status == 0)
    {
        wattron(win, COLOR_PAIR(2));
        wprintw(win, "  [SUCCESS] ");
        wattroff(win, COLOR_PAIR(2));
    }
    else
    {
        wattron(win, COLOR_PAIR(3));
        wprintw(win, "  [ERROR] ");
        wattroff(win, COLOR_PAIR(3));
    }

    wprintw(win, "%s\n\n", res->message);
    wrefresh(win);
}

return_code_t start_client_ui(int sock)
{
    return_code_t rc = ERR_OK;
    int run = 1;
    int n_choices = ARRAY_SIZE(choices);
    int highlight = 0;
    int choice = -1;
    int input_y = 10;
    int hist_start_y = 13;

    request_t req;
    response_t res;
    WINDOW *hist_win = NULL;

    if (sock < 0)
        rc = ERR_INVALID_ARG;

    if (rc == ERR_OK)
    {
        initscr();
        clear();
        noecho();
        cbreak();
        keypad(stdscr, TRUE);
        curs_set(0);

        start_color();
        init_pair(1, COLOR_BLACK, COLOR_WHITE);
        init_pair(2, COLOR_GREEN, COLOR_BLACK);
        init_pair(3, COLOR_RED, COLOR_BLACK);
        init_pair(4, COLOR_CYAN, COLOR_BLACK);

        hist_win = newwin(LINES - hist_start_y, COLS, hist_start_y, 0);
        scrollok(hist_win, TRUE);

        init_ui_layout(hist_start_y);
        wrefresh(hist_win);
    }

    while (run && rc == ERR_OK)
    {
        for (int i = 0; i < n_choices; ++i)
        {
            if (highlight == i)
            {
                attron(COLOR_PAIR(1));
                mvprintw(4 + i, 2, " %s ", choices[i]);
                attroff(COLOR_PAIR(1));
            }
            else
            {
                mvprintw(4 + i, 2, " %s ", choices[i]);
            }
        }
        refresh();

        int c = getch();
        switch (c)
        {
        case KEY_UP:
            highlight = (highlight == 0) ? n_choices - 1 : highlight - 1;
            break;
        case KEY_DOWN:
            highlight = (highlight == n_choices - 1) ? 0 : highlight + 1;
            break;
        case 10:
            choice = highlight;
            break;
        default:
            break;
        }

        if (choice != -1)
        {
            memset(&req, 0, sizeof(request_t));
            char cmd_name[100] = {0};
            char cmd_args[200] = {0};

            switch (choice)
            {
            case 0:
                req.cmd = CMD_ADD_USER;
                strcpy(cmd_name, "Add user");
                get_tui_input(input_y, 0, "New user name: ", req.username, MAX_NAME_LEN);
                snprintf(cmd_args, sizeof(cmd_args), "%s", req.username);
                break;
            case 1:
                req.cmd = CMD_DEL_USER;
                strcpy(cmd_name, "Delete user");
                get_tui_input(input_y, 0, "User name: ", req.username, MAX_NAME_LEN);
                snprintf(cmd_args, sizeof(cmd_args), "%s", req.username);
                break;
            case 2:
                req.cmd = CMD_ADD_PET;
                strcpy(cmd_name, "Add pet");
                get_tui_input(input_y, 0, "Owner name: ", req.username, MAX_NAME_LEN);
                get_tui_input(input_y + 1, 0, "Pet name: ", req.petname, MAX_NAME_LEN);
                snprintf(cmd_args, sizeof(cmd_args), "Owner: %s, Pet: %s", req.username, req.petname);
                break;
            case 3:
                req.cmd = CMD_DEL_PET;
                strcpy(cmd_name, "Delete pet");
                get_tui_input(input_y, 0, "Owner name: ", req.username, MAX_NAME_LEN);
                get_tui_input(input_y + 1, 0, "Pet name: ", req.petname, MAX_NAME_LEN);
                snprintf(cmd_args, sizeof(cmd_args), "Owner: %s, Pet: %s", req.username, req.petname);
                break;
            case 4:
                req.cmd = CMD_GET_ALL;
                strcpy(cmd_name, "Show all");
                break;
            case 5:
                req.cmd = CMD_EXIT;
                run = 0;
                break;
            }

            move(input_y + 1, 0);
            clrtoeol();

            return_code_t net_rc = send_request_and_receive(sock, &req, &res);

            if (net_rc != ERR_OK)
            {
                res.status = -1;
                strcpy(res.message, "Critical network error. Connection dropped.");
                log_history(hist_win, cmd_name, cmd_args, &res);

                wprintw(hist_win, "Press any key to exit...\n");
                wrefresh(hist_win);
                getch();
                rc = net_rc;
                run = 0;
            }
            else if (req.cmd != CMD_EXIT)
            {
                log_history(hist_win, cmd_name, cmd_args, &res);
            }

            choice = -1;
        }
    }

    if (hist_win != NULL)
    {
        delwin(hist_win);
        endwin();
    }

    return rc;
}
