// compile with:
// gcc `pkg-config --cflags gtk+-3.0` -o fontchooser_example gtk3fontsel3.c `pkg-config --libs gtk+-3.0`
#include <gtk/gtk.h>
#include <pango/pango.h>
#include <string.h>
#include <stdio.h>
GtkListBox *style_listbox = NULL; // for font styles

void populate_styles(const PangoFontFamily *family) {
    gtk_list_box_prepend(GTK_LIST_BOX(style_listbox), NULL); // clear
    GList *children, *iter;
    children = gtk_container_get_children(GTK_CONTAINER(style_listbox));
    for (iter = children; iter != NULL; iter = g_list_next(iter))
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    g_list_free(children);

    PangoFontFace **faces;
    int n_faces = 0;
    pango_font_family_list_faces(family, &faces, &n_faces);
    for (int i = 0; i < n_faces; ++i) {
        GtkWidget *row_label = gtk_label_new(pango_font_face_get_face_name(faces[i]));
        GtkWidget *row = gtk_list_box_row_new();
        gtk_container_add(GTK_CONTAINER(row), row_label);
        gtk_list_box_insert(style_listbox, row, -1);
        gtk_widget_show_all(GTK_WIDGET(style_listbox));
    }
}

void on_font_family_selected(GtkListBox *box, GtkListBoxRow *row, gpointer user_data) {
    if (!row) return;
    const char *fontname = gtk_label_get_text(GTK_LABEL(gtk_bin_get_child(GTK_BIN(row))));
    PangoFontMap *fontmap = pango_cairo_font_map_get_default();
    PangoFontFamily **families;
    int n_families = 0;
    pango_font_map_list_families(fontmap, &families, &n_families);
    for (int i = 0; i < n_families; ++i) {
        const char *family_name = pango_font_family_get_name(families[i]);
        if (!pango_font_family_is_monospace(families[i]))
            continue;
        
        if (pango_font_family_is_monospace(families[i]) &&
            strcmp(pango_font_family_get_name(families[i]), fontname) == 0) {
            populate_styles(families[i]);
            break;
        }
    }
    g_free(families);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Monospaced Font/Style Selector");
    gtk_window_set_default_size(GTK_WINDOW(window), 640, 480);

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_container_add(GTK_CONTAINER(window), hbox);

    GtkListBox *font_listbox = GTK_LIST_BOX(gtk_list_box_new());
    gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(font_listbox), TRUE, TRUE, 8);
    style_listbox = GTK_LIST_BOX(gtk_list_box_new());
    gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(style_listbox), TRUE, TRUE, 8);

    PangoFontMap *fontmap = pango_cairo_font_map_get_default();
    PangoFontFamily **families;
    int n_families = 0;
    pango_font_map_list_families(fontmap, &families, &n_families);

    for (int i = 0; i < n_families; ++i) {
        if (!pango_font_family_is_monospace(families[i])) continue;
        // Only include if the name contains the target keywords
        const char *family_name = pango_font_family_get_name(families[i]);
        if (!(strstr(family_name, "Mono") ||
              strstr(family_name, "Courier") ||
              strstr(family_name, "Code") ||
              strstr(family_name, "Anonymous") ||              
              strstr(family_name, "Typewriter") ||              
              strstr(family_name, "Mon") ||                            
              strstr(family_name, "Consol"))) {
              continue;
        }

        GtkWidget *row_label = gtk_label_new(pango_font_family_get_name(families[i]));
        GtkWidget *row = gtk_list_box_row_new();
        gtk_container_add(GTK_CONTAINER(row), row_label);
        gtk_list_box_insert(font_listbox, row, -1);
    }
    g_free(families);

    g_signal_connect(font_listbox, "row-selected", G_CALLBACK(on_font_family_selected), NULL);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}

