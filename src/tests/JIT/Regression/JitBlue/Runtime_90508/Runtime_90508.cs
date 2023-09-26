// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System.Runtime.CompilerServices;
using System.Runtime.Intrinsics;
using System.Runtime.Intrinsics.X86;
using Xunit;

public class Runtime_90508
{
    [MethodImpl(MethodImplOptions.NoInlining)]
    private static Vector128<double> Div1(Vector128<double> v, double b) => 
        v + Sse3.MoveAndDuplicate(Vector128.CreateScalarUnsafe(b));

    [MethodImpl(MethodImplOptions.NoInlining)]
    private static Vector128<double> Div2(Vector128<double> v, double b) => 
        v + Sse3.MoveAndDuplicate(Vector128.CreateScalarUnsafe(1.0));

    [MethodImpl(MethodImplOptions.NoInlining)]
    private static Vector128<double> Div3(Vector128<double> v, double b) => 
        v + Sse3.MoveAndDuplicate(Vector128.Create(1.0));

    [Fact]
    public static int TestEntryPoint()
    {
        if (!Ssse3.IsSupported)
        {
            return 100;
        }

        if (Div1(Vector128.Create(42.0), 1).ToString().Equals("<43, 43>") &&
            Div2(Vector128.Create(42.0), 1).ToString().Equals("<43, 43>") &&
            Div3(Vector128.Create(42.0), 1).ToString().Equals("<43, 43>"))
        {
            return 100;
        }
        return 101;
    }
}
