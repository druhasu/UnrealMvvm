// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/ViewModelProperty.h"
#include "Mvvm/Impl/Property/PropertyFactory.h"

template<typename TOwner, typename TValue>
class TViewModelProperty;

class FViewModelPropertyBase;
class UBaseViewModel;
class UClass;

namespace UnrealMvvm_Impl
{

    namespace Details
    {
        /* Helper class to provide GetCastedProperty method */
        template <typename TOwner, typename TValue>
        struct TBaseOperation : public FViewModelPropertyOperations
        {
            using TDecayedValue = typename TDecay<TValue>::Type;

            const TViewModelProperty<TOwner, TValue>* GetCastedProperty() const
            {
                return static_cast<const TViewModelProperty<TOwner, TValue>*>(Property);
            }
        };

        /* Implementation of GetValue method */
        template <typename TBaseOp, typename TOwner, typename TValue, bool bOptional>
        struct TGetValueOperation;

        template <typename TBaseOp, typename TOwner, typename TValue>
        struct TGetValueOperation<TBaseOp, TOwner, TValue, true> : public TBaseOp
        {
            void GetValue(UBaseViewModel* InViewModel, void* OutValue, bool& OutHasValue) const override
            {
                check(InViewModel);
                check(OutValue);

                TValue Value = this->GetCastedProperty()->GetValue((TOwner*)InViewModel);
                OutHasValue = Value.IsSet();
                if (OutHasValue)
                {
                    *((typename TBaseOp::TDecayedValue::ElementType*)OutValue) = Value.GetValue();
                }
            }
        };

        template <typename TBaseOp, typename TOwner, typename TValue>
        struct TGetValueOperation<TBaseOp, TOwner, TValue, false> : public TBaseOp
        {
            void GetValue(UBaseViewModel* InViewModel, void* OutValue, bool& OutHasValue) const override
            {
                check(InViewModel);
                check(OutValue);

                *((typename TBaseOp::TDecayedValue*)OutValue) = this->GetCastedProperty()->GetValue((TOwner*)InViewModel);
                OutHasValue = true;
            }
        };

        /* Implementation of SetValue method */
        template <typename TBaseOp, typename TOwner, typename TValue, bool bOptional>
        struct TSetValueOperation;

        template <typename TBaseOp, typename TOwner, typename TValue>
        struct TSetValueOperation<TBaseOp, TOwner, TValue, false> : public TBaseOp
        {
            void SetValue(UBaseViewModel* InViewModel, void* InValue, bool InHasValue) const override
            {
                check(InViewModel);
                check(InValue);

                this->GetCastedProperty()->SetValue((TOwner*)InViewModel, *((typename TBaseOp::TDecayedValue*)InValue));
            }
        };

        template <typename TBaseOp, typename TOwner, typename TValue>
        struct TSetValueOperation<TBaseOp, TOwner, TValue, true> : public TBaseOp
        {
            void SetValue(UBaseViewModel* InViewModel, void* InValue, bool InHasValue) const override
            {
                check(InViewModel);
                check(InValue);

                if (InHasValue)
                {
                    this->GetCastedProperty()->SetValue((TOwner*)InViewModel, typename TBaseOp::TDecayedValue(*(typename TBaseOp::TDecayedValue::ElementType*)InValue));
                }
                else
                {
                    this->GetCastedProperty()->SetValue((TOwner*)InViewModel, typename TBaseOp::TDecayedValue());
                }
            }
        };

        /* Implementation of AddClassProperty method and ContainsObjectReference method */
        template <typename TBaseOp, typename TOwner, typename TValue>
        struct TAddClassPropertyOperation : public TBaseOp
        {
            void AddClassProperty(UClass* TargetClass) const override
            {
                check(TargetClass);

                const TViewModelProperty<TOwner, TValue>* Prop = this->GetCastedProperty();
                if (Prop->GetFieldOffset() > 0)
                {
                    TPropertyFactory<typename TDecay<TValue>::Type>::AddProperty(TargetClass, Prop->GetFieldOffset(), Prop->GetName());
                }
            }

            bool ContainsObjectReference(bool bIncludeNoFieldProperties) const override
            {
                return TPropertyFactory<typename TDecay<TValue>::Type>::ContainsObjectReference &&
                    (this->GetCastedProperty()->GetFieldOffset() > 0 || bIncludeNoFieldProperties);
            }
        };

        /* Implementation of GetViewModelClass method */
        template <typename TBaseOp, typename TOwner, typename TValue>
        struct TGetViewModelClassOperation : public TBaseOp
        {
            UClass* GetViewModelClass() const override
            {
                return TOwner::StaticClass();
            }
        };

        /* Implementation of GetValueClass method */
        template <typename TBaseOp, typename TOwner, typename TValue, bool IsObject>
        struct TGetValueClassOperation;

        template <typename TBaseOp, typename TOwner, typename TValue>
        struct TGetValueClassOperation<TBaseOp, TOwner, TValue, false> : public TBaseOp
        {
            UClass* GetValueClass() const override
            {
                return nullptr;
            }
        };

        template <typename TBaseOp, typename TOwner, typename TValue>
        struct TGetValueClassOperation<TBaseOp, TOwner, TValue, true> : public TBaseOp
        {
            UClass* GetValueClass() const override
            {
                return TRemoveObjectPointer<typename TRemovePointer<TValue>::Type>::Type::StaticClass();
            }
        };
    }

    template <typename TBaseOp>
    struct TViewModelPropertyOperations : public TBaseOp
    {
        TViewModelPropertyOperations(const FViewModelPropertyBase* Prop)
        {
            check(Prop);
            this->Property = Prop;
        }
    };

}
