# Tracing and diagnostics

Switches that print what a title does, for bring-up and bug reports. They are environment variables; set them before starting the executable.

## Log timestamps

Lines written through the log macros (`[file.cpp:123 function] ...`, `LogMacros.hpp`) start with the local time when `APS5_LOG_TIMESTAMPS` is set to anything except an empty string or `0`:

```
[14:03:27.481] [Open.cpp:194 sceKernelOpen] path=/app0/...
```

The time comes from the system clock, so lines from different libraries of one process, and the output of other tools, can be lined up. Without the switch the lines are unchanged. Output that a trace prints directly to `stderr` is not stamped.

## Thread names

The host threads that run GPU work are named for debuggers and profilers: the video-out presenter (`presenter`) and the AGC queue workers (`queue 0x<n> worker`). The name is the role passed to `RaiseWorkerThreadPriority`, set whether or not `APS5_THREAD_PRIORITY` asks for a higher priority. On Linux the kernel keeps 15 characters, so the queue name is cut short there.

## `APS5_TRACE_*` switches

Each switch is read from the environment the first time the code that uses it runs. Unless the last column says otherwise, any value turns it on, an empty one included. `APS5_TRACE_FRAMES`, `APS5_TRACE_SECONDS` and `APS5_TRACE_DIR` belong to the performance trace described in [BUILD.md](BUILD.md#performance-trace); the other samplers and profilers (`APS5_PROFILE_*`, Tracy, the worker sampler) are described there too. The files listed are where the variable is read; the output is printed next to them, most of it behind a `[tag]` prefix.

| Switch | Read in | Value |
|---|---|---|
| `APS5_TRACE_AFFINITY` | `libc/include/CpuTopology.hpp` | Any value turns it on. |
| `APS5_TRACE_AJM` | `libSceAjm.native/src/Ajm.cpp` | Any value turns it on. |
| `APS5_TRACE_APR` | `libkernel/Apr/src/Apr.cpp` | Any value turns it on. |
| `APS5_TRACE_AUDIOOUT` | `libSceAudioOut/src/AudioOut.cpp` | Any value turns it on. |
| `APS5_TRACE_AUDIOOUT2` | `libSceAudioOut/src/AudioOut2Context.cpp` | Any value turns it on. |
| `APS5_TRACE_BARRIERS` | `libSceAgcDriver/Graphics/src/Recorder.cpp` | A decimal count: the number of barriers to trace. |
| `APS5_TRACE_BDA` | `recompiler/Optimization/src/ResourceTracker.cpp` | Any value turns it on. |
| `APS5_TRACE_BIGBUF` | `libSceAgcDriver/Graphics/src/GuestBufferMemory.cpp` | Any value turns it on. |
| `APS5_TRACE_BINDLESS` | `recompiler/Optimization/src/ResourceMaterializer.cpp` | Any value turns it on. |
| `APS5_TRACE_BUFFER_WRITTEN` | `recompiler/Optimization/src/DescriptorBindingBuilder.cpp` | Any value turns it on. |
| `APS5_TRACE_CAPSYNC` | `libSceAgcDriver/Execution/src/Driver/Memory/WriteEvidence.cpp`, `libSceAgcDriver/Graphics/src/Recorder.cpp` | Any value turns it on. |
| `APS5_TRACE_CONTEXT_STATE` | `libSceAgcDriver/Execution/src/Pm4.cpp` | Any value turns it on. |
| `APS5_TRACE_COPY_HLE` | `libSceAgcDriver/Execution/src/Driver/Memory/BufferCopy.cpp` | Any value turns it on. |
| `APS5_TRACE_DCC_KEYS` | `libSceAgcDriver/Execution/src/DisplayBuffer.cpp`, `libSceAgcDriver/Execution/src/Driver/Memory/BufferFill.cpp`, `libSceAgcDriver/Graphics/src/DccMetadata.cpp`, and 2 more | Any value turns it on. |
| `APS5_TRACE_DIR` | `libSceAgcDriver/Execution/src/PerformanceTrace.cpp` | A directory: where the performance trace and `aps5-summary.jsonl` are written (default: the working directory). See [Performance trace](BUILD.md#performance-trace). |
| `APS5_TRACE_DISPATCH_CACHE` | `libSceAgcDriver/Execution/src/Driver/Dispatch/Dispatch.cpp` | Any value turns it on. |
| `APS5_TRACE_DISPATCH_IO` | `libSceAgcDriver/Execution/src/Driver/Dispatch/Dispatch.cpp`, `libSceAgcDriver/Execution/src/VulkanDevice.cpp`, `libSceAgcDriver/Graphics/src/ShaderResources.cpp` | Any value turns it on; a value starting with `2` also prints the dispatch user data words. |
| `APS5_TRACE_DRAWS` | `libSceAgcDriver/Execution/src/Driver/Packets/PacketExecution.cpp`, `libSceAgcDriver/Execution/src/VulkanDevice.cpp`, `libSceAgcDriver/Graphics/src/Draw.cpp` | Any value turns it on. |
| `APS5_TRACE_EXIT` | `libc/src/Process.cpp`, `libc/src/specifics/windows/CrashReport.cpp` | Any value turns it on. |
| `APS5_TRACE_FIBER` | `libSceFiber/Export.cpp` | Any value turns it on. |
| `APS5_TRACE_FILL_COVER` | `libSceAgcDriver/Graphics/src/Texture.cpp` | Any value turns it on. |
| `APS5_TRACE_FLUSH` | `libSceAgcDriver/Graphics/src/Texture.cpp` | Any value turns it on. |
| `APS5_TRACE_FRAMES` | `libSceAgcDriver/Execution/include/PerformanceTrace.hpp`, `libSceAgcDriver/Execution/src/PerformanceTrace.cpp` | `<start>:<count>`: records `<count>` presents from present `<start>`; not together with `APS5_TRACE_SECONDS`. See [Performance trace](BUILD.md#performance-trace). |
| `APS5_TRACE_GPU` | `libSceAgcDriver/Execution/src/DisplayBuffer.cpp`, `libSceAgcDriver/Execution/src/Driver/Packets/PacketExecution.cpp`, `libSceAgcDriver/Execution/src/Driver/Queues/QueueWorker.cpp`, and 2 more | Any value turns it on. |
| `APS5_TRACE_HOST_IMPORT` | `libSceAgcDriver/Graphics/src/GuestBufferMemory.cpp` | Any value turns it on. |
| `APS5_TRACE_INDIRECT_DRAWS` | `libSceAgcDriver/Execution/src/Driver/Draw/DrawPrecheck.cpp` | Any value turns it on. |
| `APS5_TRACE_LABEL` | `libSceAgc/Cb/src/Sync.cpp`, `libSceAgcDriver/Execution/src/Driver/Synchronization/LabelHistory.cpp` | A guest address in hexadecimal: reports the `RELEASE_MEM` recorded for that address. |
| `APS5_TRACE_LATE_LABELS` | `libSceAgcDriver/Execution/src/Driver/Synchronization/WaitMemory.cpp` | Any value turns it on. |
| `APS5_TRACE_MEMORY` | `libkernel/DirectMemory/DirectMemory.cpp` | Any value turns it on. |
| `APS5_TRACE_QUERY` | `libSceAgcDriver/Execution/src/GuestMemory.cpp` | Any value turns it on. |
| `APS5_TRACE_SECONDS` | `libSceAgcDriver/Execution/include/PerformanceTrace.hpp`, `libSceAgcDriver/Execution/src/PerformanceTrace.cpp` | `<start>:<length>` in seconds; not together with `APS5_TRACE_FRAMES`. See [Performance trace](BUILD.md#performance-trace). |
| `APS5_TRACE_SHADER_REGS` | `libSceAgcDriver/Execution/src/Driver/Shaders/ShaderRegistry.cpp` | `all`, or the code address of one shader in hexadecimal. |
| `APS5_TRACE_SHADOW` | `libSceAgcDriver/Graphics/src/UnitShadow.cpp` | A decimal count: the first n publishes are traced, with the first 64 seeds and every evict and retire. |
| `APS5_TRACE_SKIPS` | `libSceAgcDriver/Execution/src/Driver/Shaders/ShaderDiagnostics.cpp` | Any value turns it on. |
| `APS5_TRACE_SSE4A` | `libc/src/specifics/windows/CrashReport.cpp` | Any value turns it on. |
| `APS5_TRACE_STAGING` | `libSceAgcDriver/Graphics/src/GuestBufferMemory.cpp` | Any value turns it on. |
| `APS5_TRACE_SYNC` | `libkernel/EventFlag/src/EventFlag.cpp` | Any value turns it on. |
| `APS5_TRACE_TEXTURES` | `libSceAgcDriver/Graphics/src/ShaderResources.cpp` | Any value turns it on. |
| `APS5_TRACE_TIMERS` | `libkernel/Time/Time.cpp` | Any value turns it on. |
| `APS5_TRACE_UNREADABLE` | `libSceAgcDriver/Execution/src/GuestMemory.cpp` | Any value turns it on. |
| `APS5_TRACE_UPLOAD` | `libSceAgcDriver/Graphics/src/Texture.cpp` | Any value turns it on. |
| `APS5_TRACE_USLEEP` | `libkernel/Time/Time.cpp` | Any value turns it on. |
| `APS5_TRACE_VIDEODEC` | `libSceVideodec2/Export.cpp` | Any value turns it on. |
| `APS5_TRACE_WAITS` | `libkernel/Time/Time.cpp` | Any value turns it on. |
