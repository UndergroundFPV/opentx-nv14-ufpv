
class Choice : public Window {
  public:
    Choice(Window * parent, const rect_t & rect, const char * values, int16_t vmin, int16_t vmax,
           std::function<int16_t()> getValue, std::function<void(int16_t)> setValue, LcdFlags flags = 0
    ) :
      Window(parent, rect),
      values(values),
      vmin(vmin),
      vmax(vmax),
      getValue(getValue),
      setValue(setValue)
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
      drawTextAtIndex(dc, 3, 3, values, getValue(), textColor);
      drawSolidRect(dc, 0, 0, rect.w, rect.h, 1, lineColor);
    }

    bool onTouch(coord_t x, coord_t y)
    {
      int16_t value = getValue() + 1;
      if (value > vmax)
        value = vmin;
      setValue(value);
      setFocus();
      return true;
    }

    const char * label;
    const char * values;
    int16_t vmin;
    int16_t vmax;
    std::function<int16_t()> getValue;
    std::function<void(int16_t)> setValue;
};
