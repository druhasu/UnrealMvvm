// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

template<typename T>
struct TPointerToMember;

template<typename TValueType, typename TClassType>
struct TPointerToMember<TValueType TClassType::*>
{
    using ValueType = TValueType;
    using ClassType = TClassType;
};