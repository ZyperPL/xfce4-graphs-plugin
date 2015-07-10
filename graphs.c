#include <libxfce4panel/xfce-panel-plugin.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TIMEOUT 200
#define SIZE 28

static void constructor(XfcePanelPlugin *plugin);
XFCE_PANEL_PLUGIN_REGISTER_INTERNAL(constructor);

struct Graph
{
  char *data;
};

static unsigned long long lastTotalUser, lastTotalUserLow, lastTotalSys, lastTotalIdle;
static unsigned long long lastReceiveBytes = 0, lastTransmitBytes = 0, maxReceiveBytes = 1, maxTransmitBytes = 1;

gboolean draw(gpointer data)
{
  GtkWidget *image = (GtkWidget*)data;
  GdkPixbuf *pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(image));
  char *pixels = gdk_pixbuf_get_pixels(pixbuf);
  for(int i=1;i<SIZE;++i)
  {
    for(int j=0;j<SIZE; ++j)
    {
      pixels[(i-1+j*SIZE)*4+0] = pixels[(i+j*SIZE)*4+0];
      pixels[(i-1+j*SIZE)*4+1] = pixels[(i+j*SIZE)*4+1];
      pixels[(i-1+j*SIZE)*4+2] = pixels[(i+j*SIZE)*4+2];
      pixels[(i-1+j*SIZE)*4+3] = pixels[(i+j*SIZE)*4+3];
    }
  }

  FILE* file;

  double percent;
  unsigned long long totalUser, totalUserLow, totalSys, totalIdle, total;
  //cpu
  /*
  file = fopen("/proc/stat", "r");
  fscanf(file, "cpu %Ld %Ld %Ld %Ld", &totalUser, &totalUserLow,
      &totalSys, &totalIdle);
  fclose(file);

  if (totalUser < lastTotalUser || totalUserLow < lastTotalUserLow ||
    totalSys < lastTotalSys || totalIdle < lastTotalIdle)
  {
    percent = 0.0;
  }
  else
  {
    total = (totalUser - lastTotalUser) + (totalUserLow - lastTotalUserLow) +
            (totalSys - lastTotalSys);
    percent = total;
    total += (totalIdle - lastTotalIdle);
    percent /= total;
    percent *= 100;
  }

  lastTotalUser = totalUser;
  lastTotalUserLow = totalUserLow;
  lastTotalSys = totalSys;
  lastTotalIdle = totalIdle;

  //printf("%f%%\n", percent);

  int i=SIZE-1;
  {
    int height = SIZE-(int)((float)SIZE*(float)round(percent)/100);
    //printf("%d\n", height);
    for(int j=0;j<SIZE; ++j)
    {
      pixels[(i+j*SIZE)*4+0] = 0;
      pixels[(i+j*SIZE)*4+1] = 0;
      pixels[(i+j*SIZE)*4+2] = 0;
      pixels[(i+j*SIZE)*4+3] = 0;

      if (j >= height)
      {
        pixels[(i+j*SIZE)*4+0] = 255;



        pixels[(i+j*SIZE)*4+3] = 230;
      }
    }
  }
  //*/

  //net receive (eth0)

  unsigned long long receiveBytes = 0, transmitBytes = 0; 
  int ret = 0;
  file = fopen("/proc/net/dev", "r");
  if (!file) fprintf(stderr, "Error: Cannot open file!\n");
  char buff[512];
  if (file != NULL)
  {
    while (fgets(buff, 512, file))
    {
      int ret = sscanf(buff, "\teth0: %Ld", &receiveBytes);
      if (ret > 0) break;
    }
  }
  fclose(file);

  if (receiveBytes == lastReceiveBytes  || lastReceiveBytes <= 0)
  {
    percent = 0.0;
  }
  else
  {
    total = abs(lastReceiveBytes - receiveBytes);
    percent = (double)(total*(1000/TIMEOUT));

    if (total > maxReceiveBytes)
    {
      maxReceiveBytes = total;
      //printf("set max: %Ld (%Ld)\n", maxReceiveBytes, total);
    }

    percent /= maxReceiveBytes;
    percent *= ((float)TIMEOUT/1000.0)*100.0;

    //printf("%f%% > %Ld (diff: %Ld)\n", percent, maxReceiveBytes, total);
  }
  

  lastReceiveBytes = receiveBytes;

  int i=SIZE-1;
  {
    int height = SIZE-(int)((float)SIZE*(float)round(percent)/100);
    //printf("%d\n", height);
    for(int j=0;j<SIZE; ++j)
    {
      pixels[(i+j*SIZE)*4+0] = 0;
      pixels[(i+j*SIZE)*4+1] = 0;
      pixels[(i+j*SIZE)*4+2] = 0;
      pixels[(i+j*SIZE)*4+3] = 0;

      if (j >= height)
      {
        pixels[(i+j*SIZE)*4+0] = 100;
        pixels[(i+j*SIZE)*4+1] = 160;
        pixels[(i+j*SIZE)*4+2] = 255;



        pixels[(i+j*SIZE)*4+3] = 230;
      }
    }
  }



  gtk_image_set_from_pixbuf(GTK_IMAGE(image), pixbuf);
  return TRUE;
}

static gboolean
widget_set_size(XfcePanelPlugin *plugin, int size, GtkWidget *widget)
{
  int border_width;
  size /= xfce_panel_plugin_get_nrows(plugin);

  border_width = size > 26*2 ? 2 : 1;

  if (xfce_panel_plugin_get_orientation(plugin) == GTK_ORIENTATION_HORIZONTAL)
    gtk_widget_set_size_request(GTK_WIDGET(widget), -1, size);
  else
    gtk_widget_set_size_request(GTK_WIDGET(widget), size, -1);

  gtk_container_set_border_width(GTK_CONTAINER (plugin), border_width);
  return TRUE;
}

static void constructor(XfcePanelPlugin *plugin)
{
  struct Graph *g = malloc(sizeof(struct Graph));
  g->data = malloc(SIZE*SIZE*4);
  memset(g->data, 63, SIZE*SIZE*4);

  GdkPixbuf *pixbuf;
  pixbuf = gdk_pixbuf_new_from_data(g->data, GDK_COLORSPACE_RGB, TRUE, 8, SIZE, SIZE, SIZE*4, NULL, NULL);

  GtkWidget *image = gtk_image_new();
  gtk_image_set_from_pixbuf(GTK_IMAGE(image), pixbuf);

  g_timeout_add(TIMEOUT, draw, image);

  gtk_container_add(GTK_CONTAINER(plugin), image);

  gtk_widget_show_all(image);
  widget_set_size(plugin, SIZE, image);
  xfce_panel_plugin_set_expand(XFCE_PANEL_PLUGIN(plugin), FALSE);
}
