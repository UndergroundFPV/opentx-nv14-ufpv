#define GET_DEFAULT(value)      [=]() -> int32_t { return value; }
#define SET_DEFAULT(value)      [=](int32_t newValue) -> void { value = newValue; }
#define GET_SET_DEFAULT(value)  GET_DEFAULT(value), SET_DEFAULT(value)
#define GET_INVERTED(value)     [=]() -> uint8_t { return !value; }
#define SET_INVERTED(value)     [=](uint8_t newValue) -> void { value = !newValue; }
#define GET_SET_INVERTED(value) GET_INVERTED(value), SET_INVERTED(value)
