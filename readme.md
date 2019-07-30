Demonstrate hange after RequestReJIT called, when Microsoft.NET.Sdk.Web used.

To reproduce: 
 - open solution in Visual Studio
 - build both projects
 - before opening set CORECLR_PATH env (relative path to Profiler\ClrProfiler.vcxproj by default ..\..\coreclr)
 - update profiler path in AppToBeInstrumented\Properties\launchSettings.json (replace C:\\projects\\dotnet\\ProfilerReJitHang with path to repo)
 - start AppToBeInstrumented debug (or use dotnet run)

App will hang after "RequestReJit success" line. Expected result: "GetReJITParameters" line after, then output every 2 seconds
If SDK will be changed to "Microsoft.NET.Sdk" issue will go away.
 