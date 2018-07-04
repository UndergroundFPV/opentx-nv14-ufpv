
class NumberEdit : public Window {
  public:
    NumberEdit(Window * parent, const rect_t & rect, int32_t vmin, int32_t vmax, int32_t step,
               std::function<int32_t()> getValue, std::function<void(int32_t)> setValue, LcdFlags flags = 0,
               const char * prefix = nullptr, const char * suffix = nullptr) :
      Window(parent, rect),
      vmin(vmin),
      vmax(vmax),
      step(step),
      getValue(getValue),
      setValue(setValue),
      flags(flags),
      prefix(prefix),
      suffix(suffix)
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
      drawNumber(dc, 3, 3, getValue(), textColor | flags, 0, prefix, suffix);
      drawSolidRect(dc, 0, 0, rect.w, rect.h, 1, lineColor);
    }

    bool onTouch(coord_t x, coord_t y)
    {
      int32_t value = getValue() + step;
      if (value > vmax)
        value = vmin;
      setValue(value);
      setFocus();
      return true;
    }

    int32_t vmin;
    int32_t vmax;
    int32_t step;
    std::function<int32_t()> getValue;
    std::function<void(int32_t)> setValue;
    LcdFlags flags;
    const char * prefix;
    const char * suffix;
};

