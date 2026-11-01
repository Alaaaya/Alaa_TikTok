// Alaa_TikTok - Instagram-style floating glass tab bar for TikTok
#import <UIKit/UIKit.h>
#import <objc/message.h>
#import <objc/runtime.h>
#import <AVFoundation/AVFoundation.h>
#import <MetalKit/MetalKit.h>
static char kBtnsKey;

static const NSInteger kGlassTag = 0x61A55;
static const NSInteger kSelTag   = 0x61A56;

#define kPrefEnabled  @"alaa_tt_glass_enabled"
#define kPrefFloating @"alaa_tt_glass_floating"
#define kPrefUnder    @"alaa_tt_video_under"
#define kPrefLabels   @"alaa_tt_hide_labels"
#define kPrefStyle    @"alaa_tt_glass_style_v2"   // 0 liquid glass, 1 dark blur, 2 light blur

static NSUserDefaults *D(void) { return NSUserDefaults.standardUserDefaults; }
static BOOL PB(NSString *k, BOOL def) { id v = [D() objectForKey:k]; return v ? [v boolValue] : def; }
#define TTGEnabled()  PB(kPrefEnabled, YES)
#define TTGFloating() PB(kPrefFloating, YES)
#define TTGUnder()    PB(kPrefUnder, YES)
#define TTGNoLabels() PB(kPrefLabels, YES)
static NSInteger TTGStyle(void) { return [D() integerForKey:kPrefStyle]; }
static void TTGRefreshAll(void);

@interface UITabBar (TTGlass)
- (void)ttg_applyGlass;
- (void)ttg_longPress:(UILongPressGestureRecognizer *)g;
- (void)ttg_showMenu;
- (void)ttg_reopen;
@end

static void TTGCapsule(UIView *v, BOOL on) {
    if (!on || ![v respondsToSelector:NSSelectorFromString(@"setCornerConfiguration:")]) return;
    Class cc = NSClassFromString(@"UICornerConfiguration");
    id cfg = nil;
    for (NSString *n in @[@"capsuleConfiguration", @"capsule"]) {
        SEL sl = NSSelectorFromString(n);
        if ([cc respondsToSelector:sl]) { cfg = ((id (*)(id, SEL))objc_msgSend)(cc, sl); break; }
    }
    if (cfg) ((void (*)(id, SEL, id))objc_msgSend)(v, NSSelectorFromString(@"setCornerConfiguration:"), cfg);
}

static UIVisualEffect *TTGEffect(void) {
    NSInteger st = TTGStyle();
    if (st == 1) return [UIBlurEffect effectWithStyle:UIBlurEffectStyleSystemThinMaterialDark];
    if (st == 2) return [UIBlurEffect effectWithStyle:UIBlurEffectStyleSystemUltraThinMaterialLight];
    Class glassCls = NSClassFromString(@"UIGlassEffect"); // iOS 26 Liquid Glass
    if (glassCls) {
        id e = [glassCls respondsToSelector:@selector(effectWithStyle:)] ? ((id (*)(id, SEL, NSInteger))objc_msgSend)(glassCls, @selector(effectWithStyle:), 1) : [[glassCls alloc] init]; // 1 = clear glass
        if ([e respondsToSelector:@selector(setTintColor:)])
            ((void (*)(id, SEL, id))objc_msgSend)(e, @selector(setTintColor:), [UIColor colorWithWhite:0 alpha:0.12]);
        if ([e respondsToSelector:@selector(setInteractive:)])
            ((void (*)(id, SEL, BOOL))objc_msgSend)(e, @selector(setInteractive:), YES);
        return e;
    }
    return [UIBlurEffect effectWithStyle:UIBlurEffectStyleSystemThinMaterialDark];
}

static BOOL TTGIsButton(UIView *v) {
    if (v.tag == kGlassTag || v.tag == kSelTag) return NO;
    if ([v isKindOfClass:[UIControl class]]) return YES;
    NSString *n = NSStringFromClass([v class]);
    return [n containsString:@"Button"];
}

static BOOL TTGIsSelected(UIView *b) {
    if ([b isKindOfClass:[UIControl class]] && ((UIControl *)b).isSelected) return YES;
    for (NSString *k in @[@"isSelected", @"selected", @"isCurrent"]) {
        SEL s = NSSelectorFromString(k);
        if ([b respondsToSelector:s]) {
            @try { if ([[b valueForKey:k] boolValue]) return YES; } @catch (__unused id e) {}
        }
    }
    return NO;
}

static void TTGCollectButtons(UIView *v, NSMutableArray *out, int d) {
    if (d > 4) return;
    for (UIView *s in v.subviews) {
        if (s.tag == kGlassTag || s.tag == kSelTag) continue;
        if (TTGIsButton(s) && s.bounds.size.width >= 30 && s.bounds.size.width <= 160) [out addObject:s];
        else TTGCollectButtons(s, out, d + 1);
    }
}
static void TTGClearBG(UIView *v) {
    for (UIView *s in v.subviews) {
        if (TTGIsButton(s)) continue;
        if ([s isKindOfClass:[UIImageView class]] && s.bounds.size.width > 200) s.hidden = YES;
        else if (s.bounds.size.width > 200) { s.backgroundColor = UIColor.clearColor; TTGClearBG(s); }
    }
}

static void TTGSetLabelsHidden(UIView *v, BOOL hidden) {
    for (UIView *s in v.subviews) {
        if ([s isKindOfClass:[UILabel class]]) {
            NSString *t = ((UILabel *)s).text;
            // keep badge counters (digits) visible
            BOOL isBadge = t.length && [t rangeOfCharacterFromSet:NSCharacterSet.decimalDigitCharacterSet.invertedSet].location == NSNotFound;
            if (!isBadge) s.alpha = hidden ? 0 : 1;
        } else TTGSetLabelsHidden(s, hidden);
    }
}

// ---------- pushing the feed under the bar ----------
static void TTGExtend(UIView *v, CGFloat barTop, CGFloat bottom, UIView *skip, int depth) {
    if (!v || depth > 9 || v == skip || v.hidden) return;
    UIWindow *w = v.window; if (!w) return;
    for (UIView *s in v.subviews) {
        if (s == skip || s.hidden || [s isKindOfClass:[UITabBar class]]) continue;
        CGRect r = [s convertRect:s.bounds toView:w];
        if (r.size.height > w.bounds.size.height * 0.4 && fabs(CGRectGetMaxY(r) - barTop) < 4) {
            CGRect f = s.frame;
            f.size.height += (bottom - barTop);
            s.frame = f;
        }
        TTGExtend(s, barTop, bottom, skip, depth + 1);
    }
}

%hook UITabBar

- (void)didMoveToWindow {
    %orig;
    BOOL has = NO;
    for (UIGestureRecognizer *g in self.gestureRecognizers) if ([g.name isEqualToString:@"Alaa_TikTok"]) has = YES;
    if (!has) {
        UILongPressGestureRecognizer *lp = [[UILongPressGestureRecognizer alloc] initWithTarget:self action:@selector(ttg_longPress:)];
        lp.name = @"Alaa_TikTok";
        lp.minimumPressDuration = 0.9;
        lp.cancelsTouchesInView = NO;
        [self addGestureRecognizer:lp];
    }
    if (!TTGEnabled()) return;
    UITabBarAppearance *ap = [UITabBarAppearance new];
    [ap configureWithTransparentBackground];
    ap.shadowColor = UIColor.clearColor;
    self.standardAppearance = ap;
    if (@available(iOS 15.0, *)) self.scrollEdgeAppearance = ap;
    self.translucent = YES;
    self.backgroundImage = [UIImage new];
    self.shadowImage = [UIImage new];
}

- (void)layoutSubviews {
    %orig;
    [self ttg_applyGlass];
}

%new
- (void)ttg_longPress:(UILongPressGestureRecognizer *)g {
    if (g.state != UIGestureRecognizerStateBegan) return;
    [[[UIImpactFeedbackGenerator alloc] initWithStyle:UIImpactFeedbackStyleMedium] impactOccurred];
    [self ttg_showMenu];
}

%new
- (void)ttg_showMenu {
    UIViewController *top = self.window.rootViewController;
    while (top.presentedViewController) top = top.presentedViewController;
    if (!top) return;
    NSArray *styles = @[@"Liquid Glass", @"ضبابي داكن", @"ضبابي فاتح"];
    UIAlertController *ac = [UIAlertController alertControllerWithTitle:@"Alaa_TikTok"
        message:@"إعدادات الشريط\n(الإطفاء الكامل بيتطبّق بعد ما تسكّر TikTok وترجع تفتحه)" preferredStyle:UIAlertControllerStyleActionSheet];
    __weak UITabBar *ws = self;
    void (^toggle)(NSString *, NSString *, BOOL) = ^(NSString *title, NSString *key, BOOL def) {
        BOOL on = PB(key, def);
        [ac addAction:[UIAlertAction actionWithTitle:[NSString stringWithFormat:@"%@ %@", on ? @"✅" : @"⬜️", title]
            style:UIAlertActionStyleDefault handler:^(UIAlertAction *a) {
                [D() setBool:!on forKey:key]; TTGRefreshAll(); [ws ttg_reopen]; }]];
    };
    toggle(@"الشريط الزجاجي", kPrefEnabled, YES);
    toggle(@"كبسولة عائمة (متل انستا)", kPrefFloating, YES);
    toggle(@"الفيديو تحت الشريط", kPrefUnder, YES);
    toggle(@"إخفاء الأسماء (أيقونات بس)", kPrefLabels, YES);
    [ac addAction:[UIAlertAction actionWithTitle:[NSString stringWithFormat:@"🎨 النمط: %@", styles[MAX(0, MIN(2, TTGStyle()))]]
        style:UIAlertActionStyleDefault handler:^(UIAlertAction *a) {
            [D() setInteger:(TTGStyle() + 1) % 3 forKey:kPrefStyle];
            [[ws viewWithTag:kGlassTag] removeFromSuperview];
            TTGRefreshAll(); [ws ttg_reopen]; }]];
    [ac addAction:[UIAlertAction actionWithTitle:@"📋 نسخ هيكل الواجهة (للمطوّر)" style:UIAlertActionStyleDefault handler:^(UIAlertAction *a) {
        NSMutableString *out = [NSMutableString string];
        UIWindow *w = ws.window;
        __block void (^dump)(UIView *, int);
        __block __weak void (^wdump)(UIView *, int);
        wdump = dump = ^(UIView *v, int d) {
            if (d > 30 || out.length > 12000) return;
            CGRect r = [v convertRect:v.bounds toView:w];
            BOOL inBar = [v isDescendantOfView:ws];
            if (r.size.height >= 300 || inBar || d < 3 || (CGRectGetMaxY(r) > w.bounds.size.height - 220 && r.size.height >= 18 && r.size.width >= 120))
                [out appendFormat:@"%@%@ %.0f,%.0f %.0fx%.0f%@\n", [@"" stringByPaddingToLength:d withString:@" " startingAtIndex:0],
                    NSStringFromClass(v.class), r.origin.x, r.origin.y, r.size.width, r.size.height, v.hidden ? @" H" : @""];
            for (UIView *s in v.subviews) wdump(s, d + 1);
        };
        dump(w, 0);
        [out appendFormat:@"tabbar %@\n", NSStringFromCGRect([ws convertRect:ws.bounds toView:w])];
        UIPasteboard.generalPasteboard.string = out;
    }]];
    [ac addAction:[UIAlertAction actionWithTitle:@"تم" style:UIAlertActionStyleCancel handler:nil]];
    if (ac.popoverPresentationController) { ac.popoverPresentationController.sourceView = self; ac.popoverPresentationController.sourceRect = self.bounds; }
    [top presentViewController:ac animated:YES completion:nil];
}

- (UIView *)hitTest:(CGPoint)p withEvent:(UIEvent *)e {
    NSArray *info = objc_getAssociatedObject(self, &kBtnsKey);
    if (info && TTGEnabled() && !self.hidden && self.alpha > 0.01) {
        CGRect pill = [info[1] CGRectValue];
        if (CGRectContainsPoint(CGRectInset(pill, -4, -8), p)) {
            if (![objc_getAssociatedObject(self, @selector(ttg_tap:)) boolValue]) {
                UITapGestureRecognizer *tg = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(ttg_tap:)];
                tg.cancelsTouchesInView = YES;
                [self addGestureRecognizer:tg];
                objc_setAssociatedObject(self, @selector(ttg_tap:), @YES, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
            }
            return self;
        }
    }
    return %orig;
}

%new
- (void)ttg_tap:(UITapGestureRecognizer *)g {
    NSArray *info = objc_getAssociatedObject(self, &kBtnsKey);
    if (!info) return;
    NSArray<UIView *> *btns = info[0]; CGRect pill = [info[1] CGRectValue];
    CGFloat x = [g locationInView:self].x;
    CGFloat slotW = (pill.size.width - 12) / btns.count;
    NSInteger i = (NSInteger)floor((x - pill.origin.x - 6) / slotW);
    i = MAX(0, MIN((NSInteger)btns.count - 1, i));
    UIView *b = btns[i];
    if ([b isKindOfClass:[UIControl class]]) {
        UIControl *c = (UIControl *)b;
        [c sendActionsForControlEvents:UIControlEventTouchDown];
        [c sendActionsForControlEvents:UIControlEventTouchUpInside];
    } else {
        for (UIGestureRecognizer *r in b.gestureRecognizers) {
            if ([r isKindOfClass:[UITapGestureRecognizer class]]) {
                @try {
                    NSArray *targets = [r valueForKey:@"_targets"];
                    for (id t in targets) {
                        Ivar it = class_getInstanceVariable([t class], "_target");
                        Ivar ia = class_getInstanceVariable([t class], "_action");
                        if (!it || !ia) continue;
                        id target = object_getIvar(t, it);
                        SEL act = *(SEL *)((uint8_t *)(__bridge void *)t + ivar_getOffset(ia));
                        if (target && act) ((void (*)(id, SEL, id))objc_msgSend)(target, act, r);
                    }
                } @catch (__unused id ex) {}
            }
        }
    }
    dispatch_async(dispatch_get_main_queue(), ^{ [self setNeedsLayout]; });
}

%new
- (void)ttg_reopen {
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.35 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{ [self ttg_showMenu]; });
}

%new
- (void)ttg_applyGlass {
    UIView *glass0 = [self viewWithTag:kGlassTag];
    if (!TTGEnabled()) { glass0.hidden = YES; [self viewWithTag:kSelTag].hidden = YES; return; }
    glass0.hidden = NO;
    self.backgroundColor = UIColor.clearColor;
    self.clipsToBounds = NO;

    NSMutableArray<UIView *> *buttons = [NSMutableArray array];
    for (UIView *v in self.subviews) {
        if (v.tag == kGlassTag || v.tag == kSelTag) continue;
        if (TTGIsButton(v)) { [buttons addObject:v]; continue; }
        if ([v isKindOfClass:[UIImageView class]] || [v isKindOfClass:[UIVisualEffectView class]]) { v.hidden = YES; continue; }
        NSMutableArray *inner = [NSMutableArray array];
        TTGCollectButtons(v, inner, 0);
        if (inner.count) { [buttons addObjectsFromArray:inner]; v.backgroundColor = UIColor.clearColor; TTGClearBG(v); }
        else if (v.subviews.count == 0) v.hidden = YES;
        else v.backgroundColor = UIColor.clearColor;
    }
    [buttons sortUsingComparator:^NSComparisonResult(UIView *a, UIView *b) {
        CGFloat ax = [a.superview convertPoint:a.center toView:self].x, bx = [b.superview convertPoint:b.center toView:self].x;
        return [@(ax) compare:@(bx)]; }];

    CGFloat W = self.bounds.size.width;
    CGFloat bottomInset = self.window ? self.window.safeAreaInsets.bottom : 0;
    BOOL noLabels = TTGNoLabels();
    CGRect pill;
    if (TTGFloating()) {
        CGFloat h = 64, pw = MIN(W * 0.76, 340);
        CGFloat y = self.bounds.size.height - bottomInset - h + (bottomInset > 0 ? 12 : -6);
        if (y < 0) y = 0;
        pill = CGRectMake((W - pw) / 2, y, pw, h);
    } else {
        pill = self.bounds;
    }

    UIVisualEffectView *glass = (UIVisualEffectView *)glass0;
    if (!glass) {
        glass = [[UIVisualEffectView alloc] initWithEffect:TTGEffect()];
        glass.tag = kGlassTag;
        glass.userInteractionEnabled = NO;
        glass.clipsToBounds = (TTGStyle() != 0 || !NSClassFromString(@"UIGlassEffect"));
        glass.overrideUserInterfaceStyle = (TTGStyle() == 2) ? UIUserInterfaceStyleLight : UIUserInterfaceStyleDark;
        glass.layer.borderColor = [UIColor colorWithWhite:1 alpha:0.14].CGColor;
        [self insertSubview:glass atIndex:0];
    } else if (self.subviews.firstObject != glass) {
        [self sendSubviewToBack:glass];
    }
    glass.frame = pill;
    glass.layer.cornerRadius = TTGFloating() ? pill.size.height / 2 : 0;
    glass.layer.cornerCurve = kCACornerCurveContinuous;
    TTGCapsule(glass, TTGFloating());
    if (TTGStyle() == 0 && NSClassFromString(@"UIGlassEffect")) glass.layer.borderWidth = 0;
    glass.layer.borderWidth = TTGFloating() ? 0.6 : 0;

    // Instagram-style layout: move buttons into the pill with transforms (TikTok re-lays frames, not transforms).
    NSMutableArray *targets = [NSMutableArray array];
    CGFloat slotW = (pill.size.width - 12) / MAX((NSUInteger)1, buttons.count);
    for (NSUInteger i = 0; i < buttons.count; i++) {
        UIView *b = buttons[i];
        CGPoint c = [b.superview convertPoint:b.center toView:self];
        CGPoint t = c;
        if (TTGFloating() && buttons.count >= 3) {
            t = CGPointMake(pill.origin.x + 6 + slotW * (i + 0.5), CGRectGetMidY(pill) + (noLabels ? 8 : 0));
        }
        if (!CGAffineTransformIsIdentity(b.transform)) b.transform = CGAffineTransformIdentity;
        b.layer.sublayerTransform = CATransform3DMakeTranslation(t.x - c.x, t.y - c.y, 0);
        [targets addObject:[NSValue valueWithCGPoint:CGPointMake(t.x, CGRectGetMidY(pill))]];
    }
    for (UIView *b in buttons) TTGSetLabelsHidden(b, noLabels);
    objc_setAssociatedObject(self, &kBtnsKey, (TTGFloating() && buttons.count >= 3) ? @[[buttons copy], [NSValue valueWithCGRect:pill]] : nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);

    // highlight capsule behind the selected tab
    UIView *sel = [self viewWithTag:kSelTag];
    if (!sel) {
        Class gc = NSClassFromString(@"UIGlassEffect");
        if (gc && TTGStyle() == 0) {
            id ge = [gc respondsToSelector:@selector(effectWithStyle:)] ? ((id (*)(id, SEL, NSInteger))objc_msgSend)(gc, @selector(effectWithStyle:), 1) : [[gc alloc] init];
            if ([ge respondsToSelector:@selector(setTintColor:)])
                ((void (*)(id, SEL, id))objc_msgSend)(ge, @selector(setTintColor:), [UIColor colorWithWhite:1 alpha:0.10]);
            sel = [[UIVisualEffectView alloc] initWithEffect:ge];
        } else {
            sel = [UIView new];
            sel.backgroundColor = [UIColor colorWithWhite:1 alpha:0.16];
        }
        sel.tag = kSelTag;
        sel.userInteractionEnabled = NO;
        [self insertSubview:sel aboveSubview:glass];
    }
    NSInteger selIdx = -1;
    for (NSUInteger i = 0; i < buttons.count; i++) if (TTGIsSelected(buttons[i])) { selIdx = i; break; }
    if (selIdx < 0 && [self respondsToSelector:@selector(selectedItem)] && self.selectedItem && self.items.count == buttons.count)
        selIdx = [self.items indexOfObject:self.selectedItem];
    if (NO && selIdx >= 0 && selIdx < (NSInteger)targets.count && TTGFloating()) { // highlight removed (1.10)
        CGFloat sw = MIN(slotW + 4, 92), sh = pill.size.height - 10;
        CGPoint tc = [targets[selIdx] CGPointValue];
        sel.frame = CGRectMake(tc.x - sw / 2, pill.origin.y + 5, sw, sh);
        sel.layer.cornerRadius = sh / 2;
        sel.layer.cornerCurve = kCACornerCurveContinuous;
        TTGCapsule(sel, YES);
        sel.hidden = NO;
    } else sel.hidden = YES;
}

%end

static void TTGRefreshIn(UIView *v) {
    if ([v isKindOfClass:[UITabBar class]]) { [v setNeedsLayout]; [v layoutIfNeeded]; return; }
    for (UIView *s in v.subviews) TTGRefreshIn(s);
}
static void TTGRefreshAll(void) {
    for (UIWindow *w in UIApplication.sharedApplication.windows) TTGRefreshIn(w);
}

static BOOL TTGIsVideoView(UIView *v) {
    if ([v.layer isKindOfClass:[AVPlayerLayer class]]) return YES;
    for (CALayer *l in v.layer.sublayers) if ([l isKindOfClass:[AVPlayerLayer class]]) return YES;
    NSString *n = NSStringFromClass(v.class).lowercaseString;
    if ([n containsString:@"interact"] || [n containsString:@"overlay"] || [n containsString:@"control"] || [n containsString:@"mask"]) return NO;
    return [n containsString:@"player"] || [n containsString:@"video"] || [n containsString:@"cover"] || [n containsString:@"render"] || [v isKindOfClass:[UIImageView class]];
}

static void TTGFillFeed(UIView *v, CGFloat barTop, CGFloat fullH, int d) {
    if (d > 16) return;
    for (UIView *s in v.subviews) {
        if (s.hidden) continue;
        CGFloat ph = s.superview.bounds.size.height;
        BOOL parentOK = ph >= fullH - 4 || fabs(ph - barTop) < 4;
        if (fabs(s.frame.size.height - barTop) < 4 && fabs(s.frame.origin.y) < 2 && parentOK && TTGIsVideoView(s)) {
            CGRect f = s.frame; f.size.height = fullH; s.frame = f;
            for (UIView *p = s.superview; p && p != v.window; p = p.superview) {
                if (p.bounds.size.height >= fullH - 4) break;
                p.clipsToBounds = NO;
            }
        }
        TTGFillFeed(s, barTop, fullH, d + 1);
    }
}

%hook AWEFeedSlidingScrollView
- (void)layoutSubviews {
    %orig;
    UIView *me = (UIView *)self;
    UIWindow *w = me.window;
    if (!w || !TTGEnabled() || !TTGUnder()) return;
    CGFloat fullH = w.bounds.size.height;
    CGFloat barTop = fullH - 83;
    for (UIWindow *ww in UIApplication.sharedApplication.windows) {
        UIView *tb = nil; NSMutableArray *st = [NSMutableArray arrayWithObject:ww];
        while (st.count && !tb) { UIView *x = st.lastObject; [st removeLastObject];
            if ([x isKindOfClass:[UITabBar class]] && !x.hidden) tb = x; else [st addObjectsFromArray:x.subviews]; }
        if (tb) { barTop = [tb convertRect:tb.bounds toView:w].origin.y; break; }
    }
    if (fullH - barTop < 20) return;
    TTGFillFeed(me, barTop, fullH, 0);
}
%end

%hook AWENormalModeTabBarController
- (void)viewDidLayoutSubviews {
    %orig;
    UIViewController *vc = (UIViewController *)self;
    if (![vc respondsToSelector:@selector(tabBar)] || !TTGEnabled()) return;
    UITabBar *tb = [(id)vc tabBar];
    UIView *root = vc.view;
    if (!tb || !root.window) return;
    UIView *barHost = tb;
    while (barHost.superview && barHost.superview != root) barHost = barHost.superview;
    barHost.backgroundColor = UIColor.clearColor;
    tb.superview.backgroundColor = UIColor.clearColor;
    if (!TTGUnder()) return;
    UIWindow *w = root.window;
    CGFloat barTop = [tb convertRect:tb.bounds toView:w].origin.y;
    CGFloat bottom = w.bounds.size.height;
    if (bottom - barTop < 20) return;
    TTGExtend(root, barTop, bottom, barHost, 0);
    root.backgroundColor = UIColor.clearColor;
    [root bringSubviewToFront:barHost];
}
%end

// ---- 1.7: stretch the feed player into the area under the floating bar ----
static CGFloat TTGBarH(void) { return 83.0; }
static CGRect TTGStretch(UIView *v, CGRect f) {
    if (!TTGEnabled() || !TTGUnder()) return f;
    CGFloat full = UIScreen.mainScreen.bounds.size.height;
    CGFloat barTop = full - TTGBarH();
    if (fabs(f.size.height - barTop) < 4) f.size.height = full;
    return f;
}
static void TTGUnclip(UIView *v) {
    CGFloat full = UIScreen.mainScreen.bounds.size.height;
    for (UIView *p = v.superview; p; p = p.superview) {
        if (p.bounds.size.height >= full - 4) break;
        if (p.clipsToBounds) p.clipsToBounds = NO;
    }
}

%hook AWEPlayVideoPlayerControllerBackgroundView
- (void)setFrame:(CGRect)f { %orig(TTGStretch((UIView *)self, f)); }
- (void)layoutSubviews { %orig; if (TTGEnabled() && TTGUnder()) TTGUnclip((UIView *)self); }
%end
%hook TTKFeedPlayerContainerView
- (void)setFrame:(CGRect)f { %orig(TTGStretch((UIView *)self, f)); }
%end
%hook TTPlayerView
- (void)setFrame:(CGRect)f { %orig(TTGStretch((UIView *)self, f)); }
%end
%hook TTMetalView
- (void)setFrame:(CGRect)f { %orig(TTGStretch((UIView *)self, f)); }
%end
%hook MTKView
- (void)setFrame:(CGRect)f { if ([((UIView *)self).superview isKindOfClass:NSClassFromString(@"TTMetalView")]) %orig(TTGStretch((UIView *)self, f)); else %orig; }
%end
%hook TTKMediaBusinessCoverImageView
- (void)setFrame:(CGRect)f { %orig(TTGStretch((UIView *)self, f)); }
%end


// ---- 1.12: discover the visible right-hand action column, not private button classes ----
static char kActionGlassKey, kActionStyleKey, kActionScanKey;
static const NSInteger kActionGlassTag = 0x61A57;

static NSString *TTGActionText(UIView *view) {
    NSMutableString *text = [NSMutableString string];
    for (NSString *key in @[@"accessibilityLabel", @"accessibilityIdentifier", @"imageNameString"]) {
        SEL selector = NSSelectorFromString(key);
        if (![view respondsToSelector:selector]) continue;
        @try {
            id value = ((id (*)(id, SEL))objc_msgSend)(view, selector);
            if ([value isKindOfClass:NSString.class]) [text appendFormat:@" %@", value];
        } @catch (__unused NSException *exception) {}
    }
    return text;
}

static BOOL TTGContainsAny(NSString *text, NSArray<NSString *> *words) {
    for (NSString *word in words) if ([text containsString:word]) return YES;
    return NO;
}

static BOOL TTGActionExcluded(NSString *text) {
    return TTGContainsAny(text.lowercaseString, @[@"avatar", @"follow", @"music", @"disc", @"record", @"download",
        @"profilbild", @"folgen", @"musik", @"متابعة", @"موسيقى", @"الصورة الشخصية"]);
}

static BOOL TTGActionNamed(NSString *text) {
    NSString *name = [text stringByFoldingWithOptions:NSDiacriticInsensitiveSearch locale:nil].lowercaseString;
    return TTGContainsAny(name, @[@"like", @"digg", @"heart", @"comment", @"collect", @"favorite", @"favourite",
        @"bookmark", @"save", @"share", @"forward", @"gefallt mir", @"kommentar", @"favoriten", @"speichern",
        @"teilen", @"اعجاب", @"تعليق", @"مفضلة", @"مفضله", @"مشاركة", @"مشاركه", @"حفظ"]);
}

static BOOL TTGCountBelow(UIView *view, UIView *icon, UIWindow *window, NSUInteger depth) {
    if (depth > 4 || view.tag == kActionGlassTag || view.hidden || view.alpha < 0.01) return NO;
    if ([view isKindOfClass:UILabel.class]) {
        NSString *text = ((UILabel *)view).text;
        CGRect labelRect = [view convertRect:view.bounds toView:window];
        CGRect iconRect = [icon convertRect:icon.bounds toView:window];
        BOOL number = [text rangeOfCharacterFromSet:NSCharacterSet.decimalDigitCharacterSet].location != NSNotFound;
        return text.length && number && fabs(CGRectGetMidX(labelRect) - CGRectGetMidX(iconRect)) < 30 &&
            CGRectGetMidY(labelRect) >= CGRectGetMaxY(iconRect) - 5 &&
            CGRectGetMinY(labelRect) < CGRectGetMaxY(iconRect) + 38;
    }
    for (UIView *child in view.subviews)
        if (TTGCountBelow(child, icon, window, depth + 1)) return YES;
    return NO;
}

static void TTGActionDebug(NSArray<NSDictionary *> *matches, NSArray<NSString *> *diagnostics) {
    static BOOL writtenMatches = NO, writtenDiagnostic = NO;
    if (writtenMatches || (!matches.count && (writtenDiagnostic || !diagnostics.count))) return;
    if (matches.count) writtenMatches = YES;
    else writtenDiagnostic = YES;
    NSMutableString *text = [NSMutableString stringWithString:@"Alaa_TikTok 1.12 matched action views\n"];
    if (!matches.count) {
        [text appendString:@"No full action column matched yet. Right-edge candidate classes and labels follow.\n"];
        for (NSString *line in diagnostics) [text appendFormat:@"%@\n", line];
    }
    for (NSDictionary *match in matches) {
        UIView *icon = match[@"icon"], *host = match[@"host"];
        [text appendFormat:@"icon=%@ host=%@ labels=%@ mode=%@\n",
            NSStringFromClass(icon.class), NSStringFromClass(host.class), match[@"text"],
            [match[@"named"] boolValue] ? @"accessibility" : @"count-column"];
    }
    NSString *directory = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES).firstObject;
    if (directory) [text writeToFile:[directory stringByAppendingPathComponent:@"alaa_tiktok_debug.txt"]
                         atomically:YES encoding:NSUTF8StringEncoding error:nil];
}

static void TTGDecorateAction(NSDictionary *match) {
    UIView *host = match[@"host"], *icon = match[@"icon"];
    if (!host.window || ![icon isDescendantOfView:host]) return;
    UIVisualEffectView *glass = objc_getAssociatedObject(host, &kActionGlassKey);
    CGRect rect = [icon convertRect:icon.bounds toView:host];
    CGFloat diameter = MAX(44, MIN(52, MAX(rect.size.width, rect.size.height) + 10));
    if (!glass) {
        glass = [[UIVisualEffectView alloc] initWithEffect:TTGEffect()];
        glass.tag = kActionGlassTag;
        glass.userInteractionEnabled = NO;
        glass.isAccessibilityElement = NO;
        glass.accessibilityElementsHidden = YES;
        [host insertSubview:glass atIndex:0];
        objc_setAssociatedObject(host, &kActionGlassKey, glass, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        objc_setAssociatedObject(host, &kActionStyleKey, @(TTGStyle()), OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    } else if ([objc_getAssociatedObject(host, &kActionStyleKey) integerValue] != TTGStyle()) {
        glass.effect = TTGEffect();
        objc_setAssociatedObject(host, &kActionStyleKey, @(TTGStyle()), OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }
    if (host.subviews.firstObject != glass) [host sendSubviewToBack:glass];
    CGRect frame = CGRectMake(CGRectGetMidX(rect) - diameter / 2, CGRectGetMidY(rect) - diameter / 2, diameter, diameter);
    // No frame, transform, gesture, hit-test or content modifications on TikTok views.
    if (!CGRectEqualToRect(glass.frame, frame)) glass.frame = frame;
    glass.hidden = NO;
    glass.layer.cornerRadius = diameter / 2;
    glass.layer.cornerCurve = kCACornerCurveContinuous;
    BOOL native = TTGStyle() == 0 && NSClassFromString(@"UIGlassEffect") != Nil;
    glass.clipsToBounds = !native;
    glass.overrideUserInterfaceStyle = TTGStyle() == 2 ? UIUserInterfaceStyleLight : UIUserInterfaceStyleDark;
    glass.layer.borderColor = [UIColor colorWithWhite:1 alpha:0.14].CGColor;
    glass.layer.borderWidth = native ? 0 : 0.6;
    TTGCapsule(glass, YES);
}

// Bounded breadth-first discovery also handles Swift/renamed feed cell classes.
static void TTGScanActions(UIWindow *window) {
    if (!window || window.hidden) return;
    NSMutableArray<UIView *> *queue = [NSMutableArray arrayWithObject:window];
    NSMutableArray<NSDictionary *> *candidates = [NSMutableArray array];
    NSMutableArray<UIVisualEffectView *> *existing = [NSMutableArray array];
    NSMutableArray<NSString *> *diagnostics = [NSMutableArray array];
    CGFloat width = window.bounds.size.width, height = window.bounds.size.height;
    for (NSUInteger i = 0; i < queue.count && i < 3000; i++) {
        UIView *view = queue[i];
        if (view.tag == kActionGlassTag) { [existing addObject:(id)view]; continue; }
        if (view.hidden || view.alpha < 0.01 || [view isKindOfClass:UITabBar.class] ||
            [view isKindOfClass:UIVisualEffectView.class]) continue;
        NSString *className = NSStringFromClass(view.class).lowercaseString;
        if (TTGContainsAny(className, @[@"commentpanel", @"commentsheet", @"keyboard", @"alert", @"profileheader"])) continue;
        if (queue.count < 3000) [queue addObjectsFromArray:view.subviews];
        if (!TTGEnabled()) continue;
        CGRect rect = [view convertRect:view.bounds toView:window];
        CGFloat w = rect.size.width, h = rect.size.height;
        if (w < 19 || w > 60 || h < 19 || h > 62 || CGRectGetMidX(rect) < width * 0.73 ||
            CGRectGetMaxX(rect) > width + 2 || CGRectGetMidY(rect) < height * 0.15 ||
            CGRectGetMaxY(rect) > height - 85) continue;
        if (diagnostics.count < 50) [diagnostics addObject:[NSString stringWithFormat:@"%@ label=%@ rect=%@",
            NSStringFromClass(view.class), TTGActionText(view), NSStringFromCGRect(rect)]];
        BOOL iconLike = [view isKindOfClass:UIImageView.class] || [view isKindOfClass:UIControl.class] ||
            TTGContainsAny(className, @[@"icon", @"button", @"animationview"]);
        if (!iconLike || [view isKindOfClass:UILabel.class]) continue;
        UIView *host = view;
        NSMutableString *text = [NSMutableString stringWithString:TTGActionText(view)];
        NSMutableString *types = [NSMutableString stringWithString:className];
        BOOL inFeed = NO, blocked = NO;
        for (UIView *parent = view.superview; parent && parent != window; parent = parent.superview) {
            NSString *name = NSStringFromClass(parent.class).lowercaseString;
            if (TTGContainsAny(name, @[@"commentpanel", @"commentsheet", @"profileheader", @"navigationbar", @"toolbar"])) { blocked = YES; break; }
            if (TTGContainsAny(name, @[@"feed", @"aweme", @"playinteraction"])) inFeed = YES;
            if (parent.bounds.size.width <= 115 && parent.bounds.size.height <= 125 && parent.bounds.size.width >= w &&
                parent.bounds.size.height >= h && ![parent isKindOfClass:UIImageView.class]) {
                host = parent;
                [text appendFormat:@" %@", TTGActionText(parent)];
                [types appendFormat:@" %@", name];
            }
        }
        if (blocked || TTGActionExcluded([text stringByAppendingString:types])) continue;
        // UIImageView contents render under its children, so its glass must be a sibling.
        if (host == view && [view isKindOfClass:UIImageView.class]) {
            UIView *parent = view.superview;
            if (!parent || parent.bounds.size.width > 115 || parent.bounds.size.height > 150) continue;
            host = parent;
        }
        BOOL named = TTGActionNamed(text);
        BOOL count = TTGCountBelow(host, view, window, 0);
        if (!named && !count) continue;
        // AX matches need no private feed class name; geometry-only matches must
        // form a multi-item vertical column below, preventing random toolbar edits.
        [candidates addObject:@{@"icon":view, @"host":host, @"text":[text copy], @"named":@(named),
            @"count":@(count), @"feed":@(inFeed), @"rect":[NSValue valueWithCGRect:rect]}];
    }

    // Deduplicate nested image/icon/control views. Prefer AX identities, then images.
    [candidates sortUsingComparator:^NSComparisonResult(NSDictionary *a, NSDictionary *b) {
        NSInteger sa = [a[@"named"] boolValue] * 2 + [a[@"icon"] isKindOfClass:UIImageView.class];
        NSInteger sb = [b[@"named"] boolValue] * 2 + [b[@"icon"] isKindOfClass:UIImageView.class];
        return sa > sb ? NSOrderedAscending : sa < sb ? NSOrderedDescending : NSOrderedSame;
    }];
    NSMutableArray<NSDictionary *> *unique = [NSMutableArray array];
    for (NSDictionary *candidate in candidates) {
        CGRect rect = [candidate[@"rect"] CGRectValue]; BOOL duplicate = NO;
        for (NSDictionary *other in unique) {
            CGRect r = [other[@"rect"] CGRectValue];
            if (candidate[@"host"] == other[@"host"] ||
                (fabs(CGRectGetMidX(rect)-CGRectGetMidX(r)) < 18 && fabs(CGRectGetMidY(rect)-CGRectGetMidY(r)) < 22)) {
                duplicate = YES; break;
            }
        }
        if (!duplicate) [unique addObject:candidate];
    }
    NSMutableArray *matches = [NSMutableArray array];
    for (NSDictionary *candidate in unique) {
        CGRect rect = [candidate[@"rect"] CGRectValue]; NSUInteger neighbours = 0;
        for (NSDictionary *other in unique) {
            CGRect r = [other[@"rect"] CGRectValue];
            CGFloat dy = fabs(CGRectGetMidY(rect) - CGRectGetMidY(r));
            if (fabs(CGRectGetMidX(rect)-CGRectGetMidX(r)) < 22 && dy > 30 && dy < 300) neighbours++;
        }
        // At least three aligned actions, with count labels or semantic identity.
        if (neighbours >= 2 && ([candidate[@"named"] boolValue] || [candidate[@"count"] boolValue])) {
            [matches addObject:candidate];
        }
    }
    NSMutableSet *activeHosts = [NSMutableSet set];
    for (NSDictionary *match in matches) { TTGDecorateAction(match); [activeHosts addObject:match[@"host"]]; }
    for (UIVisualEffectView *glass in existing) if (![activeHosts containsObject:glass.superview]) glass.hidden = YES;
    TTGActionDebug(matches, diagnostics);
}

// No timer or constant full-tree scan. Coalesce layout/attachment events per window,
// with a 350ms cooldown. Global base hooks survive TikTok private class renaming.
static void TTGQueueActionScan(UIView *view) {
    if (view.tag == kActionGlassTag || [view isKindOfClass:UIVisualEffectView.class]) return;
    UIWindow *window = view.window;
    if (!window || window.hidden) return;
    BOOL root = [view isKindOfClass:UIWindow.class];
    NSString *name = NSStringFromClass(view.class).lowercaseString;
    BOOL feed = TTGContainsAny(name, @[@"feed", @"aweme", @"playinteraction"]);
    BOOL rightIcon = view.bounds.size.width >= 19 && view.bounds.size.width <= 115 &&
        view.bounds.size.height >= 19 && view.bounds.size.height <= 150 &&
        ([view isKindOfClass:UIControl.class] || [view isKindOfClass:UIImageView.class]);
    if (!root && !feed && !rightIcon) return;
    if ([objc_getAssociatedObject(window, &kActionScanKey) boolValue]) return;
    objc_setAssociatedObject(window, &kActionScanKey, @YES, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    __weak UIWindow *weakWindow = window;
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.08 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        UIWindow *strongWindow = weakWindow;
        if (!strongWindow) return;
        TTGScanActions(strongWindow);
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.35 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
            objc_setAssociatedObject(strongWindow, &kActionScanKey, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        });
    });
}

%hook UIView
- (void)layoutSubviews {
    %orig;
    TTGQueueActionScan(self);
}
- (void)didMoveToWindow {
    %orig;
    TTGQueueActionScan(self);
}
%end
