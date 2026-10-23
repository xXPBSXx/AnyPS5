# libSceCdlgPlayerReview technical debt

### Unknown function info

- [scePlayerReviewDialogOpen](../../../core/libs/prx/libSceCdlgPlayerReview/Export.cpp) uses a recovered 168-byte parameter layout; the 40-byte target record at byte 60 and mode semantics remain unresolved. In PPSA07057's one call the parameter is a zeroed structure with the common dialog base header, a 36-byte copy goes into the record at byte 60 and the next 32-bit field at byte 100 is set; what the fields mean is not read from the call.
- [scePlayerReviewDialogGetResult](../../../core/libs/prx/libSceCdlgPlayerReview/Export.cpp) (libSceCdlgPlayerReview) - PPSA07057 calls it once, only after its dialog status query returned 3, with a zeroed result buffer of at least 64 bytes, and treats a negative result as an error. That 3 is the finished status is an interpretation, and which result fields the title reads is not known.

### Functional

- [Player review dialog](../../../core/libs/prx/libSceCdlgPlayerReview/Export.cpp) (libSceCdlgPlayerReview) shows nothing: `scePlayerReviewDialogOpen` finishes at once and the result reports that the user canceled, as `sceWebBrowserDialogOpen` does
