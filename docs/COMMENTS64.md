Joker 64

DIAGNOSTIC ONLY. No comment cards style or feed video-transition picker is implemented. Native comments are not restyled. Earlier 62/61/60 features are retained. Real TikTok/device testing remains required. Login compatibility is still NOT implemented. Existing Alaa feed theme remains partial.

Using this build

Install the 64 deb or inject the matching standalone dylib as usual, then fully quit/reopen TikTok. Close any existing comments first. Open Joker settings, tap "تفعيل التقاط التعليقات · مرة واحدة", close Joker and open a video's actual comments within 45 seconds. Allow roughly one second for the sheet to settle. Close the comments, reopen Joker and use "مشاركة سجل التشخيص". Send joker_comments64.txt within 10 minutes. Settings do NOT need to be open while comments are visible. Arming is explicit, in-memory and not persistent across app restarts. Arm again to retry. Starting a new arm clears the previous capture; exporting does not consume a valid retained capture.

The arm label acknowledges the tap; it is not a live status timer. The export reports actual arm/filter/bounds status. If there is no retained sheet, send the file anyway and explain what appeared. An implementation using a completely custom non-UIKit list/composer renderer can still be missed. Structural discovery is not semantic proof that a candidate is comments. No invented TikTok class hooks are used.

What changed

Public UIViewController presentation, appearance and child attachment events, plus UIView window-attachment fallback, notify a cheap gated observer. When not armed, there is no discovery scan or snapshot. There is no layout/frame callback capture, display link, per-frame traversal, continuous polling or touch/gesture interception. Native presentation completion blocks and lifecycle/gesture behavior are left unchanged. A lifecycle burst is coalesced into one main-queue capture after 0.25 seconds and one finite later retry after another 0.65 seconds. The arm expires after 45 seconds; at most 12 discovery scans and 24 accepted events can occur. Callbacks from old arms are invalidated.

A weak identity baseline is taken once at arming. Newly visible, sufficiently large lower-sheet or modal structures containing a public scroll/list and composer are eligible, even if the class name is unknown. Comment-name discovery is only an additional hint and still needs substantial sheet/list geometry, so a feed comment button or 56px emoji strip cannot qualify. Known Joker settings, standard activity/alert/color/document/image picker subtrees and elevated/keyboard windows are excluded. Native children and embedded sheets are discovered without requiring modal presentation. An existing reused view that was hidden during arming is eligible when visible later.

The best scored candidate's redacted tree is held in RAM, not an event history. Feed-only or lower/equal-scored captures never replace it. The last sheet survives closing it and reopening settings. Eligibility lasts 600 seconds, with a finite memory-expiry callback. New arming clears previous data. There are no uploads. Only an explicit share writes a fresh dedicated structure file; old debug files are never read or appended. Exporting with no valid sheet does not silently capture the feed.

Privacy and bounds

All capture data is public UIKit class/selector names, numeric geometry, visibility, control/gesture states and constraints/gesture counts. No text, username, comment, title, input value, accessibility string/identifier, model, image, URL, credential, private gesture target or object description is read. Existing 63 symbol sanitization and output cap are reused. No KVC/private reflection. No screenshots of real screens.

Discovery is bounded to 1800 queue entries/visits, 16 normal-level visible windows and 96 controllers, with independent per-sheet caps of 900 nodes, depth18, queue2000 and 160000 ASCII bytes. Up to8 control targets,16 action selectors per target and8 gesture records per node retain the63 limits. Records include capture trigger, score/filter reason, relative retained age and cap flags. Even an empty result reports baseline/discovery cap status and the limits. Some geometry/bounds can exclude custom sheets, multiple windows, deeply nested or very broad renderers. The export says these are candidates, not confirmed comment identities. No claim of device performance or 60fps is made.

Analysis of the owner's complete 63 log

The entire 111495-byte file pasted_01a1a1c4-caef-7534-9313-ff6e79114c1a.txt was read, not a prefix. It contains the pre-settings snapshot and explicit-share snapshot. Both report candidateRoots=0, scanned389 and391 respectively, using the63 name heuristic. The fallback is AWEMaskWindow at430x932. Controller metadata includes TTKTabBarController, AWEFeedRootViewController, AWENewFeedTableViewController and two AWEFeedCellViewController instances, with feed/tab view structure. The explicit-share capture has presented=1 on listed controllers, but the log does not establish a real comments-list identity. The pre-settings list has presented=0.

There is no verified real comment-sheet/cell/composer/sort/reply identity in this file. Zero name candidates does not prove that no custom/unknown-name sheet could exist. The log cannot determine whether comments were not visible at capture time or whether settings access/presentation obscured or bypassed them. Neither explanation is assigned to the owner. 64 removes the requirement to open settings over comments. The source log is not bundled with the release.

Tests and limitations

Synthetic public UIKit fixtures exercise unknown-class embedded list/composer sheets, modal structures, actual full-screen public modal presentation, finite asynchronous settle/retry coalescing, the actual Arabic LTR opt-in row, hidden reused sheets, small emoji-strip rejection, baseline exclusion, feed-after-dismissal retention, fresh-file redaction canaries, public control/gesture metadata, no native geometry/target/gesture changes, retention/session expiry and discovery/output/scan caps. Simulator tests call the same production observer/scheduler functions; Logos hook wiring is source-reviewed, not executed inside real TikTok. Full existing62/61/60 fixtures still cover collapse animations, quick reversal, reduced motion, drag/rotation/reuse, native picker, art/glass/settings/media regressions.

No comments design mockup, recovered HTML, reference rendering or fake side-by-side is claimed. The original comments HTML is empty; future styling still needs actual device structure and owner reference/spec. Feed transitions remain paused. This is an evidence-gathering build, not completed cards styling.
