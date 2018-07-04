#include "window.h"

class Subtitle : public Window {
  public:
    Subtitle(Window * parent, const rect_t & rect, const char * label) :
      Window(parent, rect),
      label(strdup(label))
    {
    }

    ~Subtitle()
    {
      free(label);
    }

    void paint(BitmapBuffer * dc)
    {
      dc->drawText(0, 3, label, BOLD);
    }

  protected:
    char * label;
};

