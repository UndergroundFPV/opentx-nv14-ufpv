
class NumberEdit : public Window {
  public:
    NumberEdit(Window * parent, const rect_t & rect, int32_t vmin, int32_t vmax, int32_t step,
               std::function<int32_t()> getValue, std::function<void(int32_t)> setValue, LcdFlags flags = 0,
               const char * prefix = nullptr, const char * suffix = nullptr, const char * zeroText = nullptr);

    void paint(BitmapBuffer * dc);

    bool onTouchEnd(coord_t x, coord_t y);

    int32_t vmin;
    int32_t vmax;
    int32_t step;
    std::function<int32_t()> getValue;
    std::function<void(int32_t)> setValue;
    LcdFlags flags;
    const char * prefix;
    const char * suffix;
    const char * zeroText;
};

