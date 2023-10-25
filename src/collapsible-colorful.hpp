// Copyright 2021 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
#include <functional>  // for function
#include <memory>      // for shared_ptr, allocator
#include <utility>     // for move

#include "ftxui/component/component.hpp"  // for Checkbox, Maybe, Make, Vertical, Collapsible
#include "ftxui/component/component_base.hpp"  // for Component, ComponentBase
#include "ftxui/component/component_options.hpp"  // for CheckboxOption, EntryState
#include "ftxui/dom/elements.hpp"  // for operator|=, text, hbox, Element, bold, inverted
#include "ftxui/util/ref.hpp"  // for Ref, ConstStringRef

// almost copy-pased from ftxui::Collapsible
ftxui::Component CollapsibleColorful(ftxui::ConstStringRef label, ftxui::Component child, ftxui::Color textColor = ftxui::Color::Default, ftxui::Ref<bool> show = false)
{
  using namespace ftxui;

  class Impl : public ComponentBase {
   public:
    Impl(ConstStringRef label, Component child, Ref<bool> show, ftxui::Color textColor) : show_(show), textColor_(textColor) 
    {
      CheckboxOption opt;
      opt.transform = [textColor_ = this->textColor_](EntryState s) {   // NOLINT
        auto prefix = text(s.state ? "▼ " : "▶ ");  // NOLINT
        auto t = text(s.label);
        if (s.active) {
          t |= bold;
        }
        if (s.focused) {
          t |= inverted;
        }
        if (textColor_ != ftxui::Color::Default)        {
            t |= ftxui::color(textColor_);
        }
        return hbox({prefix, t});
      };
      Add(Container::Vertical({
          Checkbox(label, show_.operator->(), opt),
          Maybe(std::move(child), show_.operator->()),
      }));
    }
    Ref<bool> show_;
    ftxui::Color textColor_ = ftxui::Color::Default;
  };

  return Make<Impl>(std::move(label), std::move(child), show, textColor);
}
