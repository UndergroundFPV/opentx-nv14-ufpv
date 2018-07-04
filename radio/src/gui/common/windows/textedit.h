#include "window.h"
#include "keyboard.h"

uint8_t zlen(const char *str, uint8_t size);

class TextEdit : public Window {
  public:
    TextEdit(Window * parent, const rect_t &rect, char * value, uint8_t length, LcdFlags flags = 0) :
      Window(parent, rect),
      value(value),
      length(length)
    {
    }

    void paint(BitmapBuffer * dc)
    {
      bool hasFocus = this->hasFocus();
      LcdFlags textColor = 0;
      LcdFlags lineColor = CURVE_AXIS_COLOR;
      if (hasFocus) {
        textColor = TEXT_INVERTED_BGCOLOR;
        lineColor = TEXT_INVERTED_BGCOLOR;
      }
      if (!hasFocus && zlen(value, length) == 0)
        dc->drawSizedText(3, 3, "---", length, CURVE_AXIS_COLOR);
      else
        dc->drawSizedText(3, 3, value, length, ZCHAR | textColor);
      drawSolidRect(dc, 0, 0, rect.w, rect.h, 1, lineColor);
      if (hasFocus) {
        coord_t cursorPos = keyboard.getCursorPos();
        dc->drawSolidFilledRect(cursorPos + 2, 3, 2, 22, 0); // TEXT_INVERTED_BGCOLOR);
      }
    }

    void onFocusLost()
    {
      keyboard.disable();
    }

    bool onTouch(coord_t x, coord_t y)
    {
      setFocus();
      if (keyboard.getField() != value) {
        keyboard.setField(value, length, parent);
      }
      keyboard.setCursorPos(x);
      return true;
    }

  protected:
    char * value;
    uint8_t length;
};
