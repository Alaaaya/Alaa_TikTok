// Treat untrusted XML as bounded pose data, never as an executable action language.
#include "MascotActions.inc"
@interface TTPoseXML : NSObject<NSXMLParserDelegate>
@property(nonatomic,strong) NSMutableDictionary *actions;
@property(nonatomic,strong) NSMutableArray *poses;
@property(nonatomic,copy) NSString *key;
@property(nonatomic) NSInteger depth,actionDepth,animation,animationDepth,nodes,total;
@property(nonatomic) BOOL invalid;
@end
@implementation TTPoseXML
- (void)parser:(NSXMLParser *)p didStartElement:(NSString *)e namespaceURI:(NSString *)uri qualifiedName:(NSString *)q attributes:(NSDictionary *)a {
    if(++self.depth>32 || ++self.nodes>8192){self.invalid=YES;[p abortParsing];return;}
    if([e isEqual:@"Action"] && !self.key){
        NSString *key=TTPetActionAliases()[a[@"Name"]];
        if(key){self.key=key;self.actionDepth=self.depth;self.animation=0;self.poses=[NSMutableArray new];}
    }
    if(!self.key)return;
    if([e isEqual:@"Animation"]){self.animation++;self.animationDepth=self.depth;}
    if([e isEqual:@"Pose"] && self.animation && (self.animation==1 || [self.key isEqual:@"drag"])) {
        NSString *image=a[@"Image"];
        NSInteger frame=TTPackFrameNumber(image);
        if(!frame)return;
        NSString *d=a[@"Duration"] ?: @"5";
        NSCharacterSet *digits=NSCharacterSet.decimalDigitCharacterSet.invertedSet;
        if(!d.length || [d rangeOfCharacterFromSet:digits].location!=NSNotFound)return;
        NSInteger duration=MAX(1,MIN(1000,d.integerValue));
        NSArray *anchor=[(a[@"ImageAnchor"] ?: @"64,128") componentsSeparatedByString:@","];
        NSInteger x=64,y=128;
        if(anchor.count==2){
            NSScanner *sx=[NSScanner scannerWithString:anchor[0]],*sy=[NSScanner scannerWithString:anchor[1]];
            NSInteger xx,yy;
            if([sx scanInteger:&xx] && sx.isAtEnd && [sy scanInteger:&yy] && sy.isAtEnd){x=MAX(0,MIN(256,xx));y=MAX(0,MIN(256,yy));}
        }
        if(self.poses.count>=512 || ++self.total>2048){self.invalid=YES;[p abortParsing];return;}
        [self.poses addObject:@[@(frame),@(duration),@(x),@(y)]];
    }
}
- (void)parser:(NSXMLParser *)p didEndElement:(NSString *)e namespaceURI:(NSString *)uri qualifiedName:(NSString *)q {
    if(self.key && self.depth==self.actionDepth){
        if(self.poses.count && !self.actions[self.key])self.actions[self.key]=[self.poses copy];
        self.key=nil;self.poses=nil;
    }self.depth--;
}
- (NSData *)parser:(NSXMLParser *)p resolveExternalEntityName:(NSString *)n systemID:(NSString *)s {self.invalid=YES;[p abortParsing];return nil;}
- (void)parser:(NSXMLParser *)p foundInternalEntityDeclarationWithName:(NSString *)n value:(NSString *)v {self.invalid=YES;[p abortParsing];}
- (void)parser:(NSXMLParser *)p foundExternalEntityDeclarationWithName:(NSString *)n publicID:(NSString *)u systemID:(NSString *)s {self.invalid=YES;[p abortParsing];}
@end
static NSDictionary *TTPackParseActions(NSData *data) {
    if(!data.length || data.length>1024*1024)return nil;
    NSString *text=[[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
    if(!text || [text rangeOfString:@"<!DOCTYPE" options:NSCaseInsensitiveSearch].location!=NSNotFound ||
       [text rangeOfString:@"<!ENTITY" options:NSCaseInsensitiveSearch].location!=NSNotFound)return nil;
    NSXMLParser *p=[[NSXMLParser alloc] initWithData:data];
    p.shouldResolveExternalEntities=NO;p.shouldProcessNamespaces=YES;
    TTPoseXML *d=[TTPoseXML new];d.actions=[NSMutableDictionary new];p.delegate=d;
    return [p parse] && !d.invalid ? d.actions : nil;
}
static BOOL TTPackSaveActions(NSData *data,NSString *stage) {
    NSDictionary *poses=TTPackParseActions(data);
    NSString *path=[stage stringByAppendingPathComponent:@"poses.json"];
    if(!poses || [NSFileManager.defaultManager fileExistsAtPath:path])return NO;
    NSData *json=[NSJSONSerialization dataWithJSONObject:poses options:0 error:nil];
    return [json writeToFile:path options:NSDataWritingAtomic error:nil];
}
static BOOL TTPackIsActions(NSString *path) {
    NSArray *parts=path.lowercaseString.pathComponents;
    return [parts.lastObject isEqual:@"actions.xml"] && parts.count>1 && [parts[parts.count-2] isEqual:@"conf"];
}
static NSDictionary *TTPackActions(NSString *pack) {
    if(![TTPackIDs() containsObject:pack])return @{};
    NSData *data=[NSData dataWithContentsOfFile:[[TTPackRoot() stringByAppendingPathComponent:pack] stringByAppendingPathComponent:@"poses.json"]];
    if(data.length>1024*1024)return @{};
    id map=data?[NSJSONSerialization JSONObjectWithData:data options:0 error:nil]:nil;
    // Sanitized files are app-owned; validate again to handle document edits.
    NSMutableDictionary *safe=[NSMutableDictionary new];
    if(![map isKindOfClass:NSDictionary.class])return safe;
    for(NSString *key in TTDefaultPetActions()){
        id poses=map[key];if(![poses isKindOfClass:NSArray.class] || [poses count]>512)continue;
        NSMutableArray *valid=[NSMutableArray new];
        for(id pose in poses){
            if(![pose isKindOfClass:NSArray.class] || [pose count]!=4)continue;
            BOOL ok=YES;for(id n in pose)if(![n isKindOfClass:NSNumber.class] || !isfinite([n doubleValue]))ok=NO;
            if(ok && [pose[0] integerValue]>=1 && [pose[0] integerValue]<=256 && [pose[1] integerValue]>=1 && [pose[1] integerValue]<=1000 &&
                [pose[2] doubleValue]>=0 && [pose[2] doubleValue]<=256 && [pose[3] doubleValue]>=0 && [pose[3] doubleValue]<=256)[valid addObject:pose];
        }if(valid.count)safe[key]=valid;
    }
    // Native aliases share their underlying frame data.
    if(safe[@"fall"])safe[@"thrown"]=safe[@"fall"];
    if(safe[@"sprawl"])safe[@"lie"]=safe[@"sprawl"];
    if(safe[@"grab"])safe[@"wallsit"]=safe[@"grab"];
    return safe;
}
