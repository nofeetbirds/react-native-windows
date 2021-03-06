// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "pch.h"

#include <Views/ControlViewManager.h>
#include <Views/ShadowNodeBase.h>

#include <Utils/PropertyUtils.h>

namespace react {
namespace uwp {

ControlViewManager::ControlViewManager(const std::shared_ptr<IReactInstance> &reactInstance) : Super(reactInstance) {}

folly::dynamic ControlViewManager::GetNativeProps() const {
  folly::dynamic props = Super::GetNativeProps();
  props.update(folly::dynamic::object("tabIndex", "number"));
  return props;
}
void ControlViewManager::TransferProperties(const XamlView &oldView, const XamlView &newView) {
  TransferProperty(oldView, newView, xaml::Controls::Control::FontSizeProperty());
  TransferProperty(oldView, newView, xaml::Controls::Control::FontFamilyProperty());
  TransferProperty(oldView, newView, xaml::Controls::Control::FontWeightProperty());
  TransferProperty(oldView, newView, xaml::Controls::Control::FontStyleProperty());
  TransferProperty(oldView, newView, xaml::Controls::Control::CharacterSpacingProperty());
  TransferProperty(oldView, newView, xaml::Controls::Control::IsTextScaleFactorEnabledProperty());
  TransferProperty(oldView, newView, xaml::Controls::Control::BackgroundProperty());
  TransferProperty(oldView, newView, xaml::Controls::Control::BorderBrushProperty());
  TransferProperty(oldView, newView, xaml::Controls::Control::BorderThicknessProperty());
  TransferProperty(oldView, newView, xaml::Controls::Control::PaddingProperty());
  TransferProperty(oldView, newView, xaml::Controls::Control::ForegroundProperty());
  TransferProperty(oldView, newView, xaml::Controls::Control::TabIndexProperty());
  // Control.CornerRadius is only supported on >= RS5
  if (oldView.try_as<xaml::Controls::IControl7>() && newView.try_as<xaml::Controls::IControl7>()) {
    TransferProperty(oldView, newView, xaml::Controls::Control::CornerRadiusProperty());
  }
  Super::TransferProperties(oldView, newView);
}

bool ControlViewManager::UpdateProperty(
    ShadowNodeBase *nodeToUpdate,
    const std::string &propertyName,
    const folly::dynamic &propertyValue) {
  auto control(nodeToUpdate->GetView().as<xaml::Controls::Control>());

  bool implementsPadding = nodeToUpdate->ImplementsPadding();
  bool finalizeBorderRadius{false};
  bool ret = true;

  if (control != nullptr) {
    if (TryUpdateBackgroundBrush(control, propertyName, propertyValue)) {
    } else if (TryUpdateBorderProperties(nodeToUpdate, control, propertyName, propertyValue)) {
    } else if (TryUpdateForeground(control, propertyName, propertyValue)) {
    } else if (TryUpdateCornerRadiusOnNode(nodeToUpdate, control, propertyName, propertyValue)) {
      finalizeBorderRadius = true;
    } else if (implementsPadding && TryUpdatePadding(nodeToUpdate, control, propertyName, propertyValue)) {
    } else if (propertyName == "tabIndex") {
      if (propertyValue.isNumber()) {
        auto tabIndex = propertyValue.asDouble();
        if (tabIndex == static_cast<int32_t>(tabIndex)) {
          if (tabIndex < 0) {
            control.IsTabStop(false);
            control.ClearValue(xaml::Controls::Control::TabIndexProperty());
          } else {
            control.IsTabStop(true);
            control.TabIndex(static_cast<int32_t>(tabIndex));
          }
        }
      } else if (propertyValue.isNull()) {
        control.ClearValue(xaml::Controls::Control::TabIndexProperty());
      }
    } else {
      ret = Super::UpdateProperty(nodeToUpdate, propertyName, propertyValue);
    }
  }

  if (finalizeBorderRadius && control.try_as<xaml::Controls::IControl7>()) {
    // Control.CornerRadius is only supported on >= RS5, setting borderRadius on Controls have no effect < RS5
    UpdateCornerRadiusOnElement(nodeToUpdate, control);
  }
  return ret;
}

void ControlViewManager::OnViewCreated(XamlView view) {
  // Set the default cornerRadius to 0 for Control: WinUI usually default cornerRadius to 2
  // Only works on >= RS5 becuase Control.CornerRadius is only supported >= RS5
  if (auto control = view.try_as<xaml::Controls::IControl7>()) {
    control.CornerRadius({0});
  }
}

} // namespace uwp
} // namespace react
