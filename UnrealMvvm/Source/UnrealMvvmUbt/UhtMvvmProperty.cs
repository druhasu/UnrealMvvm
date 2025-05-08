// Copyright Andrei Sudarikov. All Rights Reserved.

using System;
using EpicGames.UHT.Parsers;
using EpicGames.UHT.Tables;
using EpicGames.UHT.Tokenizer;
using EpicGames.UHT.Types;
using EpicGames.UHT.Utils;

namespace UnrealMvvm;

public class UhtMvvmProperty : UhtType
{
    public ReadOnlyMemory<UhtToken> PropertyTypeTokens { get; set; }

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

    protected override bool ResolveSelf(UhtResolvePhase phase)
    {
        bool bResult = base.ResolveSelf(phase);

        if (phase == UhtResolvePhase.Properties)
        {
            // we only want to resolve properties containing UObjects to trigger builtin TObjectPtr validation, when it is enabled
            // all other properties do not have to be valid according to UhtParser rules, so we skip them
            if (TypeMayContainUObject())
            {
                UhtPropertySettings propertySettings = new()
                {
                    Outer = Outer!,
                    SourceName = SourceName,
                    EngineName = SourceName,
                    PropertyCategory = UhtPropertyCategory.Member
                };

                // ask default property parser to do all the heavy work
                UhtPropertyParser.ResolveProperty(UhtPropertyResolvePhase.Resolving, propertySettings, Class.HeaderFile.Data.Memory, PropertyTypeTokens);
            }
        }

        return bResult;
    }

    private bool TypeMayContainUObject()
    {
        for (int i = 0; i < PropertyTypeTokens.Length; i++)
        {
            ref readonly var token = ref PropertyTypeTokens.Span[i];
            if (token.IsIdentifier() && (token.ValueStartsWith("U") || token.ValueStartsWith("A")))
            {
                return true;
            }
        }
        return false;
    }
}
