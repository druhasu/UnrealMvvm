// Copyright Andrei Sudarikov. All Rights Reserved.

using System.Text;

namespace UnrealMvvm;

static class UhtMvvmPropertyStringBuilderExtensions
{
    public static void AppendPropertyVariableType(this StringBuilder sb, UhtMvvmProperty p)
    {
        sb.Append(p.Class.SourceName);
        sb.Append("::F");
        sb.Append(p.SourceName);
        sb.Append("Property");
    }

    public static void AppendPropertyVariableName(this StringBuilder sb, UhtMvvmProperty p)
    {
        sb.Append(p.Class.SourceName);
        sb.Append("::");
        sb.Append(p.SourceName);
        sb.Append("PropertyValue");
    }

    public static void AppendPropertyGetterInvocation(this StringBuilder sb, UhtMvvmProperty p)
    {
        sb.Append(p.Class.SourceName);
        sb.Append("::");
        sb.Append(p.SourceName);
        sb.Append("Property()");
    }

    public static void AppendGetterPointer(this StringBuilder sb, UhtMvvmProperty p)
    {
        sb.Append('&');
        sb.Append(p.Class.SourceName);
        sb.Append("::Get");
        sb.Append(p.SourceName);
    }

    public static void AppendSetterPointer(this StringBuilder sb, UhtMvvmProperty p)
    {
        if (p.HasSetter)
        {
            sb.Append('&');
            sb.Append(p.Class.SourceName);
            sb.Append("::Set");
            sb.Append(p.SourceName);
        }
        else
        {
            sb.Append("nullptr");
        }
    }

    public static void AppendFieldOffset(this StringBuilder sb, UhtMvvmProperty p)
    {
        if (p.AutoField)
        {
            sb.Append("STRUCT_OFFSET(");
            sb.Append(p.Class.SourceName);
            sb.Append(", ");
            sb.Append(p.SourceName);
            sb.Append("Field");
            sb.Append(')');
        }
        else
        {
            sb.Append('0');
        }
    }

    public static void AppendGetterVisibility(this StringBuilder sb, UhtMvvmProperty p)
    {
        sb.Append("FViewModelPropertyBase::EAccessorVisibility::V_");
        sb.Append(p.GetterAccess);
    }

    public static void AppendSetterVisibility(this StringBuilder sb, UhtMvvmProperty p)
    {
        sb.Append("FViewModelPropertyBase::EAccessorVisibility::V_");
        sb.Append(p.SetterAccess);
    }
};
