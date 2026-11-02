Evidence and scope, not a claim of device compatibility

Reference exports contain OSTikTok1.3.1 for47.4 and BH++/signing/cache binaries for47.5. Inspected only, never linked, executed or copied. Source-backed mappings checked against public BHTikTok headers and Tweak.x in /tmp/tt122/publicref. Installed target headers/runtime are still required for unsupported features.

No ads uses AWEAwemeModel initWithDictionary:error and BOOL isAds. Only modeled feed ads; not a promise to remove every ad placement.
Follows-you and profile video count remain blocked. followerStatus/followStatus/visibleVideosCount strings do not prove class ABI.
Comment translation remains blocked, no proven class/selector ABI.
No watermark is partial only. AWEAwemeModel.video -> AWEVideoModel.playURL -> AWEURLModel.originURLList supports preferring playback assets, not a guarantee of watermark removal from every asset.
Read receipts and online privacy remain blocked, no proven receipt suppression hook.
Story save is supported by AWEFeedViewTemplateCell and TTKStoryDetailTableViewCell configWithModel:/configureWithModel:, TTKStoryContainerViewController.model or TTKStoryDetailContainerViewController.model.currentPlayingStory. Private viewing separately blocked.
Deleted messages, suggested accounts and hide live in feed remain blocked, no proven selector/type pair.
Auto-scroll uses AWEPlayVideoPlayerController playerWillLoopPlaying:, container.parentViewController AWENewFeedTableViewController.scrollToNextVideo. End-of-video only.
Background audio remains blocked. TTVideoEngine background GPU names do not prove working audio behavior or platform entitlement.
Always-show progress uses BOOL AWEAwemeModel progressBarVisible and progressBarDraggable.
Post date/country, sensitive warnings and comment-image/sticker saving remain blocked.
Profile image save uses AWEProfileImagePreviewView initWithFrame:image:imageURL:backgroundColor:userID:type:, UIImageView avatar.image.
Longer comments uses NSUInteger AWETextInputController maxLength, max(original,240). Client only; server may still reject.
Copy bio uses TIKTOKProfileHeaderView initWithFrame:, yy_viewController -> TIKTOKProfileHeaderViewController.user -> AWEUserModel.signature.
Watched marking remains blocked; noJustWatched strings hide an existing marker, they do not implement requested tracking.
Live auto-tap skipped as requested.

Updated after deeper reference inspection
Follows-you and profile video count now have firm receiver/getter evidence from reference replacement0x5f0e0. The unknown private three-argument config ABI is deliberately not hooked. Our implementation uses the known UIKit layoutSubviews callback on TIKTOKProfileHeaderView and runtime-signature-checked getters through TIKTOKProfileHeaderViewController.user. followerStatus1 means follows-you, visibleVideosCount is object-returning. Missing fields hide the badge rather than guess.
Feed warningLabel/warningImage on AWEPlayInteractionWarningElementView are proven zero-arg object getters from MSHookMessageEx sites0x5806c/0x5808c. Only those views are optionally hidden. Not an age-gate bypass.
Private story viewing remains unavailable; implemented story saving sends no receipt-related hooks.
No safe feed-only live-skip hook recovered. Reference hooks globally nil model transformers, deliberately not copied.
All features remain runtime/device-version dependent. CI and source guards are not substitutes for on-device tests.
