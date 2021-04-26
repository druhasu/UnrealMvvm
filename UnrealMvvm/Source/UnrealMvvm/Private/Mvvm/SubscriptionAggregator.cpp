// Copyright Andrei Sudarikov. All Rights Reserved.


class UExtendableClass : public UObject
{
};

void TestFunc(UObject* Context, FFrame& Stack, RESULT_DECL);

UFunction* FunctionFactory()
{
    auto NewFunction = new (EC_InternalUseOnlyConstructor, UExtendableClass::StaticClass(), UTF8_TO_TCHAR("TestFunc"), RF_Public | RF_Transient | RF_MarkAsNative) UFunction(
        FObjectInitializer(),
        nullptr,
        FUNC_Public | FUNC_Native,
        0
    );

    NewFunction->Bind();
    NewFunction->StaticLink();

    return NewFunction;
}

void TestFuncCaller()
{
    static bool Creator = []()
    {
        auto cl = UExtendableClass::StaticClass();
        cl->AddNativeFunction("TestFunc", &TestFunc);

        /*FClassFunctionLinkInfo LinkInfo;
        LinkInfo.CreateFuncPtr = &FunctionFactory;
        LinkInfo.FuncNameUTF8 = "TestFunc";
        cl->CreateLinkAndAddChildFunctionsToMap(&LinkInfo, 1);*/

        auto fn = FunctionFactory();
        cl->AddFunctionToFunctionMap(fn, "TestFunc");

        return true;
    }();

    auto objOfClass = NewObject<UObject>(nullptr, UExtendableClass::StaticClass());
}

void TestFunc(UObject* Context, FFrame& Stack, RESULT_DECL)
{
    P_FINISH;
    P_NATIVE_BEGIN;
    UE_LOG(LogTemp, Display, TEXT("!!!!!! Called dynamic function"));
    P_NATIVE_END;
}