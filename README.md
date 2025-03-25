# UnrealMvvm
[![MIT license](http://img.shields.io/badge/license-MIT-brightgreen.svg)](http://opensource.org/licenses/MIT)

MVVM Framework for Unreal. Provides base classes for creating View and ViewModel layers, leaving Model layer implementation to user.

You can use [UnrealDI](https://github.com/druhasu/UnrealDI) plugin in Model layer, but it is not required.

## Features

* Base C++ class for ViewModel
* Custom property framework for ViewModels, usable from both C++ and Blueprints
* Widget based View class. Available in C++ and Blueprints
* Actor based View class. Available in C++ and Blueprints
* Easy to use bindings to ViewModel properties from both C++ and Blueprints
* Stable code base covered by lots of unit tests and ready for production use

## Supported versions

I test this plugin to work with the latest version of Unreal Engine and two versions before it.

It should probably work with previous versions too, but I cannot properly test it, because Blueprint test classes are already saved with 5.3 version. It is difficult to downgrade them.

If you have any issues, please submit them [here](https://github.com/druhasu/UnrealMvvm/issues/new)

## Installation

UnrealMvvm is distributed in source form only and must be compiled together with the rest of the project

1. Download [latest release](https://github.com/druhasu/UnrealMvvm/releases/latest)
2. Extract contents of the archive into your project /Plugins/UnrealMvvm folder
3. Add "UnrealMvvm" to PrivateDependencyModuleNames inside your .Build.cs file
4. Build the project

## Getting started

To start using the plugin you need to create a ViewModel and a View.

### Creating ViewModel

To create a ViewModel you need to inherit from UBaseViewModel and define its properties.

Something like that:
```c++
#include "Mvvm/BaseViewModel.h" // base class is located here
#include "Engine/Texture2D.h" // required for storing in ViewModel property
#include "MyViewModel.generated.h"

UCLASS()
class UMyViewModel : public UBaseViewModel
{
    GENERATED_BODY()

    /* 
       Define Title property of type FText
       The macro will generate GetTitle and SetTitle methods and TitleField variable to store the value
       This property will also be bindable from Blueprints
     */
    VM_PROP_AG_AS(FText, Title, public);

    /*
       Define Icon property of type UTexture2D*
       This property will prevent GC from destroying stored object
     */
    VM_PROP_AG_AS(UTexture2D*, Icon, public);
};

```

Note `VM_PROP_AG_AS` macro. It is used to define ViewModel properties. It has following parameters:
1. Type of Property. May be a value (`FText`), a pointer (`UTexture2D*`) or a const reference (`const TArray<FString>&`). This type defines return value from Getter method.  
   If you need to use TMap here just wrap whole type name in parantheses, like this:  
   `VM_PROP_AG_AS((TMap<int32, int32>), MyMap, public, public)`.
3. Name of Property. Dictates naming of Getter, Setter and Backing field. This name is also displayed in Blueprint editor
4. Optinal Getter visibility. May be `public`, `protected` or `private`. If only one parameter is supplied, it is used for setter
5. Optional Setter visibility. May be `public`, `protected` or `private`.

There are multiple flavours of this macro allowing you to manually define Getter and Setter methods:
* `VM_PROP_AG_AS` generates both Setter and Getter bodies for you
* `VM_PROP_AG_MS` generates only Getter body and Setter must be defined in cpp file
* `VM_PROP_MG_AS` generates only Setter body and Getter must be defined in cpp file
* `VM_PROP_MG_MS` does not generate Getter and Setter bodies and both of them must be defined in cpp file

There are also _NF variants of this macro (like `VM_PROP_AG_AS_NF`). They don't generate backing field for you property and you must provide one in the header.

Finally, there is `VM_PROP_MG_NF` macro that does not generate Getter method body or backing field. It is used for creating properties that does not store anything and compute returned value inside Getter method

### Creating View

View may be implemented as Widget or Actor. They are very similar and follow the same pattern.

To create a View you need to inherit from either `UUserWidget` or `AActor` and additionally from `TBaseView<,>`.

Something like that:
```c++
#include "Blueprint/UserWidget.h"
#include "Mvvm/BaseView.h" // for TBaseView
#include "MyViewModel.h" // for our new ViewModel
#include "MyViewWidget.generated.h"

/*
   This class inherits from UUserWidget because we are creating Widget View
   This class also inherits from TBaseView<UMyViewWidget, UMyViewModel>.
     First template paramater is always the same as the class we are creating - UMyViewWidget
     Second template parameter is the type of ViewModel class that we'll use - UMyViewModel
 */
UCLASS()
class UMyViewWidget : public UUserWidget, public TBaseView<UMyViewWidget, UMyViewModel>
{
    GENERATED_BODY()

protected:
    void BindProperties() override
    {
        /* 
           Bind to TitleProperty of ViewModel.
           ViewModelType is shorthand for current ViewModel type. In our case it is UMyViewModel
           We bind the property to a lambda that sets value to a Textblock widget.
           You can also bind to a class method
         */
        Bind(this, ViewModelType::TitleProperty(), [this](const FText& Title){ Text_Title->SetText(Title); });

        // Example of binding to a method
        Bind(this, ViewModelType::IconProperty(), &ThisClass::OnIconChange);
    }

    void OnIconChange(const UTexture2D* Icon);

    // TextBlock widget to print value of TitleProperty
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    UTextBlock* Text_Title;
};

```

### Connecting View and ViewModel

After we created required classes, we can connect them together:

```c++
// Create instance of ViewModel
UMyViewModel* ViewModel = NewObject<UMyViewModel>();

// Set value to Title property
ViewModel->SetTitle(INVTEXT("Hello World!"));

// Create instance of View widget from Blueprint subclass
UMyViewWidget Widget = CreateWidget<UMyViewWidget>(MyViewWidgetBlueprintClass);

// Set ViewModel to View widget
Widget->SetViewModel(ViewModel);

// Add widget to Viewport. All bindings will be evaluated and widget will display "Hello World!" message
Widget->AddToViewport();

// Change value of Title property to something different. Widget will now display new message 
ViewModel->SetTitle(INVTEXT("Goodbye World!"));
```

## Contributing

If you have any issues, post them [here](https://github.com/druhasu/UnrealMvvm/issues)

I am only accepting pull requests from people that I know in person.
