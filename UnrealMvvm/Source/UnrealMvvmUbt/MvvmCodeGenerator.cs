// Copyright Andrei Sudarikov. All Rights Reserved.

using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using EpicGames.Core;
using EpicGames.UHT.Parsers;
using EpicGames.UHT.Tables;
using EpicGames.UHT.Tokenizer;
using EpicGames.UHT.Types;
using EpicGames.UHT.Utils;

namespace UnrealMvvm;

[UnrealHeaderTool]
public static class MvvmCodeGenerator
{
    private const string ModuleName = "UnrealMvvm";
    private const string ExporterName = "UnrealMvvmGenerator";
    private const string ExporterNameFixup = "UnrealMvvmGeneratorFixup";

    private static ConcurrentBag<UhtMvvmProperty> Properties = new();

    private static List<string> GeneratedFiles = new();

    static MvvmCodeGenerator()
    {
        // Debugging helper
        //AppDomain.CurrentDomain.FirstChanceException += (s, e) => { Log.WriteLine(LogEventType.Error, e.Exception.ToString()!); };
    }

    [UhtKeyword(Extends = UhtTableNames.Class, Keyword = "VM_PROP_AG_AS")]
    [UhtKeyword(Extends = UhtTableNames.Class, Keyword = "VM_PROP_MG_AS")]
    [UhtKeyword(Extends = UhtTableNames.Class, Keyword = "VM_PROP_AG_MS")]
    [UhtKeyword(Extends = UhtTableNames.Class, Keyword = "VM_PROP_MG_MS")]
    [UhtKeyword(Extends = UhtTableNames.Class, Keyword = "VM_PROP_AG_AS_NF")]
    [UhtKeyword(Extends = UhtTableNames.Class, Keyword = "VM_PROP_MG_AS_NF")]
    [UhtKeyword(Extends = UhtTableNames.Class, Keyword = "VM_PROP_AG_MS_NF")]
    [UhtKeyword(Extends = UhtTableNames.Class, Keyword = "VM_PROP_MG_MS_NF")]
    [UhtKeyword(Extends = UhtTableNames.Class, Keyword = "VM_PROP_MG_NF")]
    public static UhtParseResult VM_PROP_Keyword(UhtParsingScope topScope, UhtParsingScope actionScope, ref UhtToken token)
    {
#if UE_5_5_OR_LATER
        UhtMvvmProperty property = new(topScope.HeaderFile, topScope.ScopeType, token.InputLine);
#else
        UhtMvvmProperty property = new(topScope.ScopeType, token.InputLine);
#endif
        Properties.Add(property);
        topScope.ScopeType.AddChild(property);

        string macroName = token.ToString();

        property.AutoGetter = macroName.Contains("AG");
        property.AutoSetter = macroName.Contains("AS");
        property.AutoField = !macroName.Contains("NF");
        property.HasSetter = macroName.Contains("AS") || macroName.Contains("MS");

        var reader = topScope.TokenReader;

        reader.Require('(');

        ReadPropertyTypeTokens(property, reader);
        property.SourceName = ReadPropertyName(reader);

        var firstSpecifier = ReadAccessSpecifier(reader);
        var secondSpecifier = ReadAccessSpecifier(reader);

        if (firstSpecifier != null)
        {
            if (secondSpecifier != null)
            {
                property.GetterAccess = firstSpecifier;
                property.SetterAccess = secondSpecifier;
            }
            else
            {
                if (property.HasSetter)
                {
                    property.SetterAccess = firstSpecifier;
                }
                else
                {
                    property.GetterAccess = firstSpecifier;
                }
            }
        }

        reader.ConsumeUntil(')');

        if (reader.TryPeekOptional('{'))
        {
            UhtToken dummy = new();
            reader.SkipDeclaration(ref dummy);
        }
        else
        {
            reader.ConsumeUntil(';');
        }

        reader.CommitPendingComments();
        topScope.AddFormattedCommentsAsTooltipMetaData(property);
        reader.ClearComments();

        return UhtParseResult.Handled;
    }

    private static void ReadPropertyTypeTokens(UhtMvvmProperty property, IUhtTokenReader reader)
    {
        char EndSymbol = reader.TryOptional('(') ? ')' : ',';

        // UhtProperty parser expects this token to be removed from type tokens
        reader.Optional("const");

#if UE_5_6_OR_LATER
        property.PropertyTypeTokens = UhtTypeTokens.Gather(reader);
#else
        List<UhtToken> result = new();
        while (!reader.IsEOF)
        {
            if (reader.TryOptional(EndSymbol))
            {
                break;
            }

            result.Add(reader.GetToken());
        }
        property.PropertyTypeTokens = new ReadOnlyMemory<UhtToken>(result.ToArray());
#endif
        reader.Optional(')');
        reader.Optional(',');
    }

    private static string ReadPropertyName(IUhtTokenReader reader)
    {
        string result = reader.GetToken().ToString();
        reader.Optional(',');
        return result;
    }

    private static string? ReadAccessSpecifier(IUhtTokenReader reader)
    {
        if (reader.TryOptionalIdentifier(out var token))
        {
            reader.Optional(',');
            return token.ToString();
        }

        return null;
    }

    [UhtExporter(Name = ExporterName, ModuleName = ModuleName, Options = UhtExporterOptions.Default)]
    public static void GenerateCode(IUhtExportFactory factory)
    {
        WriteGeneratedCode(factory);
    }

    [UhtExporter(Name = ExporterNameFixup, ModuleName = ModuleName, Options = UhtExporterOptions.Default)]
    public static void FixupGeneratedFileNames(IUhtExportFactory factory)
    {
        foreach (var originalFilepath in GeneratedFiles)
        {
            var originalWriteTime = File.GetLastWriteTime(originalFilepath);
            var newFilepath = originalFilepath.Substring(0, originalFilepath.Length - ".keep".Length) + ".cpp";

            File.Copy(originalFilepath, newFilepath, true);
            File.SetLastWriteTime(newFilepath, originalWriteTime);
        }
    }

    private static void WriteGeneratedCode(IUhtExportFactory factory)
    {
        GeneratedFiles.Clear();

        foreach (var propertiesPerModule in Properties.GroupBy(p => p.Class.Package.Module))
        {
            var sortedIncludeFiles = propertiesPerModule
                .Select(prop => prop.Class.HeaderFile)
                .Distinct()
                .OrderBy(f => GetProperIncludePath(f))
                .ToArray();

            var sortedProperties = propertiesPerModule
                .OrderBy(p => p.Class.SourceName)
                .ThenBy(p => p.SourceName)
                .ToArray();

            using BorrowStringBuilder borrower = new(StringBuilderCache.Big);
            var sb = borrower.StringBuilder;

            sb.AppendLine("// Copyright Andrei Sudarikov. All Rights Reserved.");
            sb.AppendLine();
            sb.AppendLine("/* ------------------------------------------------ *");
            sb.AppendLine("     This file was generated by UnrealMvvm plugin");
            sb.AppendLine("     DO NOT modify it manually!");
            sb.AppendLine(" * ------------------------------------------------ */");
            sb.AppendLine();

            sb.AppendLine("#include \"Mvvm/Impl/Property/ViewModelRegistry.h\"");
            sb.AppendLine();

            foreach (UhtHeaderFile? headerFile in sortedIncludeFiles)
            {
                sb.Append("#include \"");
                sb.Append(GetProperIncludePath(headerFile));
                sb.Append("\"");
                sb.AppendLine();
            }

            sb.AppendLine();

            foreach (UhtMvvmProperty property in sortedProperties)
            {
                // Example of generated line:
                //
                //    const UMyClass::FMyProperty UMyClass::MyPropertyValue = { GetterPtr, SetterPtr, FieldOffset, EVisibility::V_##GetterVisibility, EVisibility::V_##SetterVisibility };

                sb.AppendPropertyVariableType(property);
                sb.Append(' ');
                sb.AppendPropertyVariableName(property);
                sb.Append(" = ");
                sb.Append("{ ");
                sb.AppendGetterPointer(property);
                sb.Append(", ");
                sb.AppendSetterPointer(property);
                sb.Append(", ");
                sb.AppendFieldOffset(property);
                sb.Append(", ");
                sb.AppendGetterVisibility(property);
                sb.Append(", ");
                sb.AppendSetterVisibility(property);
                sb.Append(" };");

                sb.AppendLine();
            }

            sb.AppendLine();

            sb.AppendLine("struct FViewModelPropertiesRegistrator");
            sb.AppendLine("{");
            sb.AppendLine("    FViewModelPropertiesRegistrator()");
            sb.AppendLine("    {");

            foreach (UhtMvvmProperty property in sortedProperties)
            {
                sb.Append("        UnrealMvvm_Impl::FViewModelRegistry::RegisterProperty(");

                sb.AppendPropertyGetterInvocation(property);
                sb.Append(", ");

                // Name
                sb.Append("\"");
                sb.Append(property.SourceName);
                sb.Append("\"");

                sb.AppendLine(");");
            }

            sb.AppendLine("    }");

#if UE_5_5_OR_LATER
            UhtModule module = propertiesPerModule.Key;
            string moduleName = module.Module.Name;
            string moduleOutputDirectory = module.Module.OutputDirectory;
#else
            UHTManifest.Module module = propertiesPerModule.Key;
            string moduleName = module.Name;
            string moduleOutputDirectory = module.OutputDirectory;
#endif

            sb.Append("} GViewModelPropertiesRegistrator_");
            sb.Append(moduleName);
            sb.AppendLine(";");

            var outputPath = Path.Combine(moduleOutputDirectory, $"{moduleName}.Mvvm.gen.keep");
            GeneratedFiles.Add(outputPath);

            factory.CommitOutput(outputPath, sb);
        }
    }

    private static string GetProperIncludePath(UhtHeaderFile headerFile)
    {
        if (headerFile.ModuleRelativeFilePath.StartsWith("Public", StringComparison.OrdinalIgnoreCase))
        {
            // output relative path for files in the "Public" folder
            return headerFile.IncludeFilePath;
        }
        else
        {
            // otherwise output full path with module Name
#if UE_5_5_OR_LATER
            return headerFile.Module.Module.Name + "/" + headerFile.ModuleRelativeFilePath;
#else
            return headerFile.Package.Module.Name + "/" + headerFile.ModuleRelativeFilePath;
#endif
        }
    }
}
