#include <functional>
#include "window.h"

class Button : public Window {
  public:
    Button(Window * parent, const rect_t & rect, std::function<uint8_t(void)> onPress, uint8_t state=0):
      Window(parent, rect),
      onPress(onPress),
      state(state)
    {
    }

    bool onTouch(coord_t x, coord_t y)
    {
      state = onPress();
      return true;
    }

  protected:
    std::function<uint8_t(void)> onPress;
    uint8_t state;
};

class TextButton : public Button {
  public:
    TextButton(Window * parent, const rect_t & rect, const char * label, std::function<uint8_t(void)> onPress):
      Button(parent, rect, onPress),
      label(label)
    {
    }

    void paint(BitmapBuffer * dc)
    {
      dc->drawText(rect.w / 2, 0, label, CENTERED);
      if (state)
        drawSolidRect(dc, 0, 0, rect.w, rect.h, 2, SCROLLBOX_COLOR);
      else
        drawSolidRect(dc, 0, 0, rect.w, rect.h, 1, CURVE_AXIS_COLOR);
    }

  protected:
    const char * label;
};

class IconButton: public Button {
  public:
    IconButton(Window * parent, const rect_t & rect, uint8_t icon, std::function<uint8_t(void)> onPress, uint8_t state=0):
      Button(parent, rect, onPress, state),
      icon(icon)
    {
    }

    void paint(BitmapBuffer * dc)
    {
      dc->drawBitmap(0, 0, theme->getIconBitmap(icon, state));
    }

  protected:
    uint8_t icon;
};
