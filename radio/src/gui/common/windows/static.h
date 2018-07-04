class StaticText : public Window {
  public:
    StaticText(Window * parent, const rect_t & rect, const char * text, LcdFlags flags = 0) :
      Window(parent, rect),
      flags(flags),
      text(strdup(text))
    {
    }

    ~StaticText()
    {
      free(text);
    }

    void paint(BitmapBuffer * dc)
    {
      dc->drawText(0, 3, text, flags);
    }

    LcdFlags flags;

  protected:
    char * text;
};

class StaticBitmap: public Window {
  public:
    StaticBitmap(Window * parent, const rect_t & rect, const char * filename):
      Window(parent, rect),
      bitmap(BitmapBuffer::load(filename))
    {
    }

    void paint(BitmapBuffer * dc)
    {
      dc->drawBitmap(0, 0, bitmap);
    }

    BitmapBuffer * bitmap;
};
