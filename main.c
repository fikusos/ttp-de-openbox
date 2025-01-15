#include <gtk/gtk.h>
#include <vte/vte.h>
#include <gdk/gdkx.h>

#define NUM_TABS 15

GtkWidget *window;
GtkWidget *tabs[NUM_TABS];
int current_tab = 0;
int tab_count = 1;

void set_window_below(GtkWidget *window) {
    GdkWindow *gdk_window = gtk_widget_get_window(window);
    if (gdk_window) {
        Display *xdisplay = GDK_WINDOW_XDISPLAY(gdk_window);
        Window xwindow = GDK_WINDOW_XID(gdk_window);
        Atom net_wm_state = XInternAtom(xdisplay, "_NET_WM_STATE", False);
        Atom net_wm_state_below = XInternAtom(xdisplay, "_NET_WM_STATE_BELOW", False);
        Atom net_wm_state_skip_taskbar = XInternAtom(xdisplay, "_NET_WM_STATE_SKIP_TASKBAR", False);
        Atom net_wm_state_skip_pager = XInternAtom(xdisplay, "_NET_WM_STATE_SKIP_PAGER", False);
        XClientMessageEvent xclient;
        memset(&xclient, 0, sizeof(xclient));
        xclient.type = ClientMessage;
        xclient.window = xwindow;
        xclient.message_type = net_wm_state;
        xclient.format = 32;
        xclient.data.l[0] = 1;
        xclient.data.l[1] = net_wm_state_below;
        xclient.data.l[2] = net_wm_state_skip_taskbar;
        xclient.data.l[3] = net_wm_state_skip_pager;
        xclient.data.l[4] = 0;
        XSendEvent(xdisplay, DefaultRootWindow(xdisplay), False,
                   SubstructureRedirectMask | SubstructureNotifyMask,
                   (XEvent *)&xclient);
    }
}

const char *get_shell() {
    const char *shell = g_getenv("SHELL");
    if (!shell || access(shell, X_OK) != 0) {
        shell = "/bin/bash";
        if (access(shell, X_OK) != 0) {
            shell = "/bin/sh";
        }
    }
    return shell;
}

GtkWidget *create_tab() {
    GtkWidget *terminal = vte_terminal_new();
    gtk_widget_show(terminal);
    const char *shell = get_shell();
    char *shell_argv[] = { (char *)shell, NULL };
    vte_terminal_spawn_async(VTE_TERMINAL(terminal),
                             VTE_PTY_DEFAULT,
                             NULL,
                             shell_argv,
                             NULL,
                             G_SPAWN_SEARCH_PATH,
                             NULL, NULL, NULL, -1, NULL, NULL, NULL);
    return terminal;
}

void switch_to_tab(int tab_index) {
    if (tab_index >= 0 && tab_index < tab_count) {
        gtk_widget_hide(tabs[current_tab]);
        gtk_widget_show(tabs[tab_index]);
        current_tab = tab_index;
    }
}

gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
    if (event->state & GDK_CONTROL_MASK) {
        if (event->keyval >= GDK_KEY_1 && event->keyval <= GDK_KEY_9) {
            int tab_index = event->keyval - GDK_KEY_1;
            if (tab_index < tab_count) {
                switch_to_tab(tab_index);
            } else if (tab_count < NUM_TABS) {
                tabs[tab_count] = create_tab();
                gtk_container_add(GTK_CONTAINER(window), tabs[tab_count]);
                gtk_widget_hide(tabs[tab_count]);
                tab_count++;
                switch_to_tab(tab_index);
            }
            return TRUE;
        } else if (event->keyval == GDK_KEY_0) {
            int tab_index = 9;
            if (tab_index < tab_count) {
                switch_to_tab(tab_index);
            } else if (tab_count < NUM_TABS) {
                tabs[tab_count] = create_tab();
                gtk_container_add(GTK_CONTAINER(window), tabs[tab_count]);
                gtk_widget_hide(tabs[tab_count]);
                tab_count++;
                switch_to_tab(tab_index);
            }
            return TRUE;
        }
    }
    return FALSE;
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "TTP-DE");
    gtk_window_maximize(GTK_WINDOW(window));
    gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
    gtk_window_set_keep_below(GTK_WINDOW(window), TRUE);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    set_window_below(window);
    tabs[0] = create_tab();
    gtk_container_add(GTK_CONTAINER(window), tabs[0]);
    g_signal_connect(window, "key-press-event", G_CALLBACK(on_key_press), NULL);
    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}
