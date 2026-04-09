#pragma once

#include <string>

template<typename CoordType>
class Rectangle {

    public:

    Rectangle() = default;

    Rectangle(CoordType initLeft,
              CoordType initTop,
              CoordType initWidth,
              CoordType initHeight)
    {
        left_ = initLeft;
        top_ = initTop;
        width_ = initWidth;
        height_ = initHeight;
    }

    void setLeft(CoordType l) {
        left_ = l;
    }

    CoordType getTop() const {
        return top_;
    }
    CoordType getBottom() const {
        return top_ + height_ - 1;
    }
    CoordType getLeft() const {
        return left_;
    }
    CoordType getRight() const {
        return left_ + width_ - 1;
    }
    CoordType getWidth() const {
        return width_;
    }
    CoordType getHeight() const {
        return height_;
    }
    CoordType getCenterX() const {
        return left_ + width_ / 2;
    }
    CoordType getCenterY() const {
        return top_ + height_ / 2;
    }

    void centeredResize(CoordType width, CoordType height) {
        const auto x = getCenterX();
        const auto y = getCenterY();

        left_ = x - width / 2;
        top_ = y - height / 2;

        width_ = width;
        height_ = height;
    }

    Rectangle<CoordType> removeFromTop(CoordType v) {
        Rectangle<CoordType> removed = *this;

        removed.height_ = v;

        top_ += v;
        height_ -= v;

        return removed;
    }
    Rectangle<CoordType> removeFromBottom(CoordType v) {
        Rectangle<CoordType> removed = *this;

        removed.height_ = v;
        removed.top_ = getBottom() - v;

        height_ -= v;

        return removed;
    }
    Rectangle<CoordType> removeFromLeft(CoordType v) {
        Rectangle<CoordType> removed = *this;

        removed.width_ = v;

        left_ += v;
        width_ -= v;
        return removed;
    }
    Rectangle<CoordType> removeFromRight(CoordType v) {
        Rectangle<CoordType> removed = *this;

        removed.width_ = v;
        removed.left_ = getRight() - v;

        width_ -= v;
        return removed;
    }

    Rectangle<float> toFloat() const {
        return Rectangle<float>((float)left_,
                                (float)top_,
                                (float)width_,
                                (float)height_);
    };

    Rectangle<int> toInt() const {
        return Rectangle<int>((int)left_,
                              (int)top_,
                              (int)width_,
                              (int)height_);
    };

    private:
    CoordType top_, left_, width_, height_;
};

const auto screenBounds = Rectangle<int>(0, 0, 128, 64);

void setPixel(int x, int y, bool set = true);
void drawLine(int x0, int y0, int x1, int y1, bool set = true);
void drawRect(Rectangle<int> rect, bool set = true);
void fillRect(Rectangle<int> rect, bool set = true);

int printH(int x, int y, char c, bool set = true);
int printH(int x, int y, const std::string& str, bool set = true);

int printV(int x, int y, char c, bool set = true);
int printV(int x, int y, const std::string& str, bool set = true);

int printMidiKeyV(int x, int y, uint8_t key, bool set = true);