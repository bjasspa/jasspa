// gcc `pkg-config --cflags gtk+-3.0` -o fontchooser_example gtk3fontsel4.c `pkg-config --libs gtk+-3.0`
// https://www.perplexity.ai/search/can-you-add-a-little-check-if-Vr08hyMnTGSg1u1oi1hsWQ
#include <gtk/gtk.h>
#include <pango/pango.h>
#include <string.h>
#include <stdio.h>
GtkListBox *style_listbox = NULL;
GtkLabel *preview_label = NULL;
GtkComboBoxText *size_combo = NULL;
char current_family[128] = "";
char current_style[128] = "";
char current_size[8] = "12";

// Update preview
void update_preview() {
    char font_desc[256];
    snprintf(font_desc, sizeof(font_desc), "%s %s %s", current_family, current_style, current_size);
    PangoFontDescription *desc = pango_font_description_from_string(font_desc);
    gtk_widget_override_font(GTK_WIDGET(preview_label), desc);
    gtk_label_set_text(preview_label, "The quick brown fox jumps over the lazy dog.");
    pango_font_description_free(desc);
}

// Style selection handler
void on_style_selected(GtkListBox *box, GtkListBoxRow *row, gpointer user_data) {
    if (!row) return;
    GtkWidget *child = gtk_bin_get_child(GTK_BIN(row));
    strncpy(current_style, gtk_label_get_text(GTK_LABEL(child)), sizeof(current_style)-1);
    current_style[sizeof(current_style)-1] = 0;
    update_preview();
}

// Font size selector handler
void on_size_changed(GtkComboBoxText *combo, gpointer user_data) {
    strncpy(current_size, gtk_combo_box_text_get_active_text(combo), sizeof(current_size)-1);
    current_size[sizeof(current_size)-1] = 0;
    update_preview();
}

// Font family selection handler
void on_family_selected(GtkListBox *box, GtkListBoxRow *row, gpointer user_data) {
    if (!row) return;
    const char *family_name = gtk_label_get_text(GTK_LABEL(gtk_bin_get_child(GTK_BIN(row))));
    strncpy(current_family, family_name, sizeof(current_family)-1);
    current_family[sizeof(current_family)-1] = 0;

    // Remove previous style entries
    GList *rows = gtk_container_get_children(GTK_CONTAINER(style_listbox));
    for (GList *iter = rows; iter; iter = g_list_next(iter))
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    g_list_free(rows);

    // Add new style entries
    PangoFontMap *fontmap = pango_cairo_font_map_get_default();
    PangoFontFamily **families;
    int n_families;
    pango_font_map_list_families(fontmap, &families, &n_families);
    for (int i = 0; i < n_families; ++i) {
        if (!pango_font_family_is_monospace(families[i]))
            continue;
        if (strcmp(pango_font_family_get_name(families[i]), family_name) == 0) {
            PangoFontFace **faces;
            int n_faces;
            pango_font_family_list_faces(families[i], &faces, &n_faces);
            for (int j = 0; j < n_faces; ++j) {
                GtkWidget *row_label = gtk_label_new(pango_font_face_get_face_name(faces[j]));
                GtkWidget *row = gtk_list_box_row_new();
                gtk_container_add(GTK_CONTAINER(row), row_label);
                gtk_list_box_insert(style_listbox, row, -1);
            }
            if (n_faces > 0) {
                strncpy(current_style, pango_font_face_get_face_name(faces[0]), sizeof(current_style) - 1);
                current_style[sizeof(current_style)-1] = 0;
            }
            break;
        }
    }
    g_free(families);
    gtk_widget_show_all(GTK_WIDGET(style_listbox));
    update_preview();
}
static void on_close_button_clicked(GtkButton *button, gpointer user_data) {
    GtkWindow *window = GTK_WINDOW(user_data);
    gtk_window_close(window);
}
// Main function
int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Font & Size Preview");
    gtk_window_set_default_size(GTK_WINDOW(window), 700, 500);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

    // Font family listbox
    GtkListBox *family_listbox = GTK_LIST_BOX(gtk_list_box_new());
    gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(family_listbox), TRUE, TRUE, 4);

    // Style listbox
    style_listbox = GTK_LIST_BOX(gtk_list_box_new());
    gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(style_listbox), TRUE, TRUE, 4);

    // Populate families (with substring check)
    PangoFontMap *fontmap = pango_cairo_font_map_get_default();
    PangoFontFamily **families;
    int n_families;
    pango_font_map_list_families(fontmap, &families, &n_families);

    for (int i = 0; i < n_families; ++i) {
        const char *family_name = pango_font_family_get_name(families[i]);
        if (!pango_font_family_is_monospace(families[i]))
            continue;
        if (!(strstr(family_name, "Mon") ||
              strstr(family_name, "Courier") ||
              strstr(family_name, "Code") ||
              strstr(family_name, "Inconsolata") ||              
              strstr(family_name, "Typewriter") ||              
              strstr(family_name, "Terminus") ||                            
              strstr(family_name, "Proggy") ||                            
              strstr(family_name, "Consol")))
            continue;
        GtkWidget *row_label = gtk_label_new(family_name);
        GtkWidget *row = gtk_list_box_row_new();
        gtk_container_add(GTK_CONTAINER(row), row_label);
        gtk_list_box_insert(family_listbox, row, -1);
    }
    g_free(families);

    g_signal_connect(family_listbox, "row-selected", G_CALLBACK(on_family_selected), NULL);
    g_signal_connect(style_listbox, "row-selected", G_CALLBACK(on_style_selected), NULL);

    // Font size selector
    size_combo = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
    gtk_combo_box_text_append_text(size_combo, "8");
    gtk_combo_box_text_append_text(size_combo, "9");
    gtk_combo_box_text_append_text(size_combo, "10");
    gtk_combo_box_text_append_text(size_combo, "11");
    gtk_combo_box_text_append_text(size_combo, "12");
    gtk_combo_box_text_append_text(size_combo, "14");
    gtk_combo_box_text_append_text(size_combo, "16");
    gtk_combo_box_text_append_text(size_combo, "18");
    gtk_combo_box_text_append_text(size_combo, "20");
    gtk_combo_box_set_active(GTK_COMBO_BOX(size_combo), 4);
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(size_combo), FALSE, FALSE, 0);
    g_signal_connect(size_combo, "changed", G_CALLBACK(on_size_changed), NULL);
    GtkWidget *button = gtk_button_new_with_label("Close Window");
    g_signal_connect(button, "clicked", G_CALLBACK(on_close_button_clicked), window);
    //gtk_window_set_child(GTK_WINDOW(window), button);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Preview Label
    preview_label = GTK_LABEL(gtk_label_new(""));
    gtk_label_set_justify(preview_label, GTK_JUSTIFY_CENTER);
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(preview_label), FALSE, FALSE, 8);
    gtk_box_pack_end(GTK_BOX(vbox), GTK_WIDGET(button), FALSE, FALSE, 0);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);
    gtk_main();
    printf("%s %s %s\n", current_family, current_style, current_size);
    return 0;
}
