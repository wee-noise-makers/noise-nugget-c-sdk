#pragma once

#include <string>
#include <utility>
#include "sequencer.hpp"
#include "screen.hpp"
#include "Synth.hpp"

enum GUIinput {
    Button_A,
    Button_B,
    Enc_A_Left,
    Enc_A_Right,
    Enc_B_Left,
    Enc_B_Right
};

struct GUIevent
{
    GUIinput input;
};

class View {

    public:
    virtual bool event(GUIevent event) = 0;
    virtual void draw() = 0;

    Rectangle<int> getBounds() const {
        return bounds_;
    }

    void setBounds(Rectangle<int> newBounds) {
        bounds_ = newBounds;
    }

    private:

    Rectangle<int> bounds_;
};

enum class laneSettings {
    Key = 0,
    Chan,
    Velocity,
    Count
};

template<typename EnumType>
EnumType operator++(EnumType& s, int)
{
    auto i = static_cast<int>(s);
    i = std::min(i + 1, static_cast<int>(EnumType::Count) - 1);
    s = static_cast<EnumType>(i);
    return s;
}

template<typename EnumType>
EnumType operator--(EnumType& s, int)
{
    auto i = static_cast<int>(s);
    i = std::max(i - 1, 0);
    s = static_cast<EnumType>(i);
    return s;
}

class LaneView : public View {
    public:

    LaneView(SeqLane& lane)
        : lane_(lane)
    {
    }

    void setFocus(bool focus = true) {
        focus_ = focus;
    }

    bool event(GUIevent event) {
        return true;
    }

    void draw() {
        const auto bounds = getBounds();
        auto y = bounds.getBottom();
        y = printV(bounds.getLeft(), y,
                      lane_.isOn() ? '*' : '-');

        if (focus_) {
            y = printV(bounds.getLeft(), y, "->");
        } else {
            y = printMidiKeyV(bounds.getLeft(), y,
                              lane_.getKey());
        }
    }

    void settingNext(laneSettings setting) {
        switch (setting)
        {
        case laneSettings::Key:
            lane_.nextKey();
            break;
        case laneSettings::Chan:
            lane_.nextChan();
            break;
        case laneSettings::Velocity:
            lane_.nextVelocity();
        default:
            break;
        }
    }

    void settingPrev(laneSettings setting) {
        switch (setting)
        {
        case laneSettings::Key:
            lane_.prevKey();
            break;
        case laneSettings::Chan:
            lane_.prevChan();
        case laneSettings::Velocity:
            lane_.prevVelocity();
            break;
        default:
            break;
        }
    }

    void drawEdit(Rectangle<int> editBounds, laneSettings setting, bool frame){

        const int editWidth = 8 * 8 + 2;
        int editLeft = getBounds().getCenterX() - editWidth / 2;
        int editRight = getBounds().getCenterX() + editWidth / 2;

        if (editLeft < editBounds.getLeft()) {
            editLeft = editBounds.getLeft();
            editRight = editLeft + editWidth;
        } else if (editRight > editBounds.getRight()) {
            editRight = editBounds.getRight();
            editLeft = editRight - editWidth;
        }

        editBounds = Rectangle<int>(editLeft, editBounds.getTop(), editWidth, editBounds.getHeight());

        auto valueBounds = Rectangle<int>(0,
                                          editBounds.getTop() + 2,
                                          10,
                                          editBounds.getHeight() - 4);

        if (frame) {
            drawRect(editBounds);
        }

        int x = editBounds.getLeft() + 2;
        const int y = editBounds.getBottom() - 2;

        printV(x,
               y - 3,
               "Key:");
        x += 8 + 2;

        if (setting == laneSettings::Key) {

            valueBounds.setLeft(x - 2);
            fillRect(valueBounds);
        }
        printMidiKeyV(x,
                      y - 6,
                      lane_.getKey(),
                      setting != laneSettings::Key);

        x += 8 + 4;
        printV(x,
               y - 3,
               "Chan:");
        x += 8 + 2;
        if (setting == laneSettings::Chan) {
            valueBounds.setLeft(x - 2);
            fillRect(valueBounds);
        }
        printV(x,
               y - 6,
               std::to_string(lane_.getChan() + 1),
               setting != laneSettings::Chan);

        x += 8 + 4;
        printV(x,
               y - 3,
               "Velo:");
        x += 8 + 2;
        if (setting == laneSettings::Velocity) {
            valueBounds.setLeft(x - 2);
            fillRect(valueBounds);
        }
        printV(x,
               y - 6,
               std::to_string(lane_.getVelocity()),
               setting != laneSettings::Velocity);
    }

    private:

    SeqLane& lane_;
    bool focus_ = false;
};

class LanesPage : public View {
    public:
    LanesPage(PaperSequencer &seq)
     : lanes_{seq.lanes[0],
              seq.lanes[1],
              seq.lanes[2],
              seq.lanes[3],
              seq.lanes[4],
              seq.lanes[5],
              seq.lanes[6],
              seq.lanes[7],
              seq.lanes[8],
              seq.lanes[9],
              seq.lanes[10],
              seq.lanes[11],
              seq.lanes[12],
              seq.lanes[13],
              seq.lanes[14]}
    {
        lanes_[laneFocus_].setFocus(true);
    }

    void setBounds(Rectangle<int> newBounds) {
        View::setBounds(newBounds);
        auto lanesBounds = newBounds.removeFromBottom(3 * 8);
        editBounds_ = newBounds;
        const auto laneWidth = lanesBounds.getWidth() / kSensorsLaneCount;
        for (auto& l : lanes_) {
            l.setBounds(lanesBounds.removeFromRight(laneWidth));
        }
    }

    bool event(GUIevent event) {
        if (laneEdit_) {
            LaneView& focused = lanes_[laneFocus_];

            switch (event.input)
            {
            case GUIinput::Enc_A_Left:
                laneSetting_--;
                break;

            case GUIinput::Enc_A_Right:
                laneSetting_++;
                break;

            case GUIinput::Enc_B_Left:
                focused.settingPrev(laneSetting_);
                break;

            case GUIinput::Enc_B_Right:
                focused.settingNext(laneSetting_);

            case GUIinput::Button_A:
                break;

            case GUIinput::Button_B:
                laneEdit_ = false;
                break;

            default:
                break;
            }

            return false;
        }

        switch (event.input)
        {
        case GUIinput::Enc_A_Left:
            if (laneFocus_ < kSensorsLaneCount - 1) {
                lanes_[laneFocus_].setFocus(false);
                laneFocus_ += 1;
                lanes_[laneFocus_].setFocus(true);
            }
            break;

        case GUIinput::Enc_A_Right:
            if (laneFocus_ > 0) {
                lanes_[laneFocus_].setFocus(false);
                laneFocus_ -= 1;
                lanes_[laneFocus_].setFocus(true);
            }
            break;

        case GUIinput::Enc_B_Left:
            lanes_[laneFocus_].settingPrev(laneSetting_);
            break;

        case GUIinput::Enc_B_Right:
            lanes_[laneFocus_].settingNext(laneSetting_);
            break;

        case GUIinput::Button_A:
            laneEdit_ = true;
            break;

        case GUIinput::Button_B:
            return true;
            break;

        default:
            break;
        }


        return false;
    }

    void draw() {
        for (auto& l : lanes_) {
            l.draw();
        }

        LaneView& focused = lanes_[laneFocus_];
        focused.drawEdit(editBounds_, laneSetting_, laneEdit_);
    }

    private:

    int laneFocus_ = 0;
    bool laneEdit_ = false;

    laneSettings laneSetting_ = laneSettings::Key;
    Rectangle<int> editBounds_;
    std::array<LaneView, kSensorsLaneCount> lanes_;
};

class SynthView: public View {

    public:

    SynthView(uint8_t chan)
      : ctrls_(chan)
    {
    }

    bool event(GUIevent event) {
        switch (event.input)
        {
        case GUIinput::Enc_A_Left:
            selected_--;
            break;

        case GUIinput::Enc_A_Right:
            selected_++;
            break;

        case GUIinput::Enc_B_Left:
            ctrls_.valuePrev(selected_);
            break;

        case GUIinput::Enc_B_Right:
            ctrls_.valueNext(selected_);
            break;

        case GUIinput::Button_B:
            return true;
            break;

        default:
            break;
        }

        return false;
    }

    void draw() {
        auto bounds = getBounds();
        for (int i = 0; i < PARAM_COUNT; i++) {
            const auto p = static_cast<synth_param>(i);
            const bool set = p != selected_;
            const auto pBounds = bounds.removeFromLeft(8 * 2);

            if (!set) {
                fillRect(pBounds);
            }

            printV(pBounds.getLeft(),
                   pBounds.getBottom() - 4,
                   ctrls_.paramImg(p),
                   set);

            const int valueShift = p == synth_param::Shape ? 2 : 8;
            printV(pBounds.getLeft() + 8,
                   pBounds.getBottom() - valueShift,
                   ctrls_.valueImg(p),
                   set);
        }
    }

    private:

    SynthControls ctrls_;
    synth_param selected_ = synth_param::Shape;
};

enum class MainMenus {
    Lanes = 0,
    Synth,
    Count
};

class MainView: public View {

    public:

    MainView(PaperSequencer &seq)
      : lanesPage_(seq)
      , synth_(0)
    {

    }

    void setBounds(Rectangle<int> newBounds) {
        View::setBounds(newBounds);
        lanesPage_.setBounds(newBounds);
        synth_.setBounds(newBounds);
    }

    bool event(GUIevent event) {

        if (inSub_) {
            switch (selected_)
            {
            case MainMenus::Lanes:
                inSub_ = ! lanesPage_.event(event);
                break;

            case MainMenus::Synth:
                inSub_ = ! synth_.event(event);
                break;

            default:
                break;
            }
            return false;
        }

        switch (event.input)
        {
        case GUIinput::Enc_A_Left:
            selected_--;
            break;

        case GUIinput::Enc_A_Right:
            selected_++;
            break;

        case GUIinput::Button_A:
            inSub_ = true;
            break;

        default:
            break;
        }

        return false;
    }

    void draw() {
        if (inSub_) {
            switch (selected_)
            {
            case MainMenus::Lanes:
                lanesPage_.draw();
                break;

            case MainMenus::Synth:
                synth_.draw();
                break;

            default:
                break;
            }
            return;
        } else {
            auto bounds = getBounds();

            bounds.centeredResize(bounds.getWidth() - 20, bounds.getHeight() - 10);

            const int itemSize = bounds.getWidth() / static_cast<int>(MainMenus::Count);
            const auto laneBounds = bounds.removeFromLeft(itemSize);
            const auto synthBounds = bounds.removeFromLeft(itemSize);

            if (selected_ == MainMenus::Lanes) {
                fillRect(laneBounds);
            } else {
                drawRect(laneBounds);
            }
            printV(laneBounds.getCenterX() - 4,
                   laneBounds.getCenterY() + 2 * 8 + 4,
                   "Lanes", selected_ != MainMenus::Lanes);
            if (selected_ == MainMenus::Synth) {
                fillRect(synthBounds);
            } else {
                drawRect(synthBounds);
            }
            printV(synthBounds.getCenterX() - 4,
                   synthBounds.getCenterY() + 2 * 8 + 4,
                   "Synth", selected_ != MainMenus::Synth);
        }
    }

    private:
    LanesPage lanesPage_;
    SynthView synth_;

    bool inSub_ = false;
    MainMenus selected_ = MainMenus::Lanes;
};

