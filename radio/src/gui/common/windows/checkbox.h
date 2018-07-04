
class CheckBox : public Window {
  public:
    CheckBox(Window * parent, const rect_t & rect, std::function<uint8_t()> getValue, std::function<void(uint8_t)> setValue,
             LcdFlags flags = 0) :
      Window(parent, rect),
      getValue(getValue),
      setValue(setValue)
    {
    }

    void paint(BitmapBuffer * dc)
    {
      bool hasFocus = this->hasFocus();
      uint8_t value = getValue();
      if (hasFocus) {
        dc->drawSolidFilledRect(0, 6, 14, 14, TEXT_INVERTED_BGCOLOR);
        dc->drawSolidFilledRect(2, 8, 10, 10, TEXT_BGCOLOR);
        if (value) {
          dc->drawSolidFilledRect(3, 9, 8, 8, TEXT_INVERTED_BGCOLOR);
        }
      }
      else {
        if (value) {
          dc->drawSolidFilledRect(3, 9, 8, 8, SCROLLBOX_COLOR);
          drawSolidRect(dc, 1, 7, 12, 12, 1, LINE_COLOR);
        }
        else {
          drawSolidRect(dc, 1, 7, 12, 12, 1, LINE_COLOR);
        }
      }
    }

    bool onTouch(coord_t x, coord_t y)
    {
      setValue(!getValue());
      setFocus();
      return true;
    }

    const char * label;
    std::function<uint8_t()> getValue;
    std::function<void(uint8_t)> setValue;
};
