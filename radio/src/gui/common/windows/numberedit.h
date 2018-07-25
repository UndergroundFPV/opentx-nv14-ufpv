
class NumberEdit : public Window {
  public:
    NumberEdit(Window * parent, const rect_t & rect, int32_t vmin, int32_t vmax,
               std::function<int32_t()> getValue, std::function<void(int32_t)> setValue, LcdFlags flags = 0);

    void paint(BitmapBuffer * dc);

    void setMin(int32_t value) {
      vmin = value;
    }

    void setMax(int32_t value) {
      vmax = value;
    }

    void setStep(int32_t step) {
      this->step = step;
    }

    void setPrefix(const char * prefix) {
      this->prefix = prefix;
    }

    void setSuffix(const char * suffix) {
      this->suffix = suffix;
    }

    void setZeroText(const char * text) {
      zeroText = text;
    }

    virtual void setDisplayFunction(std::function<void(BitmapBuffer *, LcdFlags, int32_t)> function) {
      displayFunction = function;
    }

    virtual bool onTouchEnd(coord_t x, coord_t y) override;

  protected:
    int32_t vmin;
    int32_t vmax;
    int32_t step = 1;
    std::function<int32_t()> getValue;
    std::function<void(int32_t)> setValue;
    std::function<void(BitmapBuffer *, LcdFlags, int32_t)> displayFunction;
    LcdFlags flags;
    const char * prefix = nullptr;
    const char * suffix = nullptr;
    const char * zeroText = nullptr;
};

