// Copyright Andrei Sudarikov. All Rights Reserved.

using EpicGames.UHT.Types;
using EpicGames.UHT.Utils;

namespace UnrealMvvm;

public class UhtMvvmProperty : UhtType
{
    public string? PropertyType { get; set; }

    public bool AutoSetter { get; set; } = false;

    public bool AutoGetter { get; set; } = false;

    public bool AutoField { get; set; } = false;

    public bool HasSetter { get; set; } = false;

    public string GetterAccess { get; set; } = "public";

    public string SetterAccess { get; set; } = "private";

    public override UhtEngineType EngineType => UhtEngineType.Property;

    public override string EngineClassName => "ViewModelProperty";

    public UhtClass Class => (Outer as UhtClass)!;

#if UE_5_5_OR_LATER
    public UhtMvvmProperty(UhtModule module)
        : base(module)
    {
    }

    public UhtMvvmProperty(UhtHeaderFile headerFile, UhtType outer, int lineNumber, UhtMetaData? metaData = null)
        : base(headerFile, outer, lineNumber, metaData)
    {
    }
#else
    public UhtMvvmProperty(UhtSession session)
        : base(session)
    {
    }

    public UhtMvvmProperty(UhtType outer, int lineNumber, UhtMetaData? metaData = null)
        : base(outer, lineNumber, metaData)
    {
    }
#endif
}
