#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import <CoreVideo/CoreVideo.h>
#import <math.h>
static void Check(BOOL ok,NSString *label) { if(!ok) { NSLog(@"FAIL %@",label);exit(1); } }
static NSURL *Temp(NSString *ext) {return [NSURL fileURLWithPath:[NSTemporaryDirectory() stringByAppendingPathComponent:[NSUUID.UUID.UUIDString stringByAppendingString:ext]]];}
static void Export(AVAsset *asset, NSURL *url, NSString *preset, AVFileType type) {
    AVAssetExportSession *s=[[AVAssetExportSession alloc] initWithAsset:asset presetName:preset];
    Check(s!=nil,@"export init");s.outputURL=url;s.outputFileType=type;
    dispatch_semaphore_t done=dispatch_semaphore_create(0);
    [s exportAsynchronouslyWithCompletionHandler:^{dispatch_semaphore_signal(done);}];
    Check(dispatch_semaphore_wait(done,dispatch_time(DISPATCH_TIME_NOW,60*NSEC_PER_SEC))==0,@"export timeout");
    Check(s.status==AVAssetExportSessionStatusCompleted,[NSString stringWithFormat:@"export %@",s.error]);
}
int main(void) { @autoreleasepool {
    NSURL *wav=Temp(@".wav"),*video=Temp(@".mp4"),*audio=Temp(@".m4a"),*muxed=Temp(@".mp4");
    uint32_t samples=44100*2,bytes=samples*2,riff=bytes+36,fmt=16,rate=44100,byteRate=88200;
    uint16_t pcm=1,channels=1,align=2,bits=16;
    NSMutableData *d=[NSMutableData data];[d appendBytes:"RIFF" length:4];[d appendBytes:&riff length:4];[d appendBytes:"WAVEfmt " length:8];
    [d appendBytes:&fmt length:4];[d appendBytes:&pcm length:2];[d appendBytes:&channels length:2];
    [d appendBytes:&rate length:4];[d appendBytes:&byteRate length:4];[d appendBytes:&align length:2];[d appendBytes:&bits length:2];
    [d appendBytes:"data" length:4];[d appendBytes:&bytes length:4];
    for(uint32_t i=0;i<samples;i++){int16_t v=(int16_t)(12000*sin(2*M_PI*440*i/44100.));[d appendBytes:&v length:2];}
    Check([d writeToURL:wav atomically:YES],@"wav fixture");
    Export([AVURLAsset URLAssetWithURL:wav options:nil],audio,AVAssetExportPresetAppleM4A,AVFileTypeAppleM4A);
    NSError *error=nil;
    AVAssetWriter *writer=[[AVAssetWriter alloc] initWithURL:video fileType:AVFileTypeMPEG4 error:&error];
    AVAssetWriterInput *input=[AVAssetWriterInput assetWriterInputWithMediaType:AVMediaTypeVideo outputSettings:@{AVVideoCodecKey:AVVideoCodecTypeH264,AVVideoWidthKey:@64,AVVideoHeightKey:@64}];
    AVAssetWriterInputPixelBufferAdaptor *adaptor=[AVAssetWriterInputPixelBufferAdaptor assetWriterInputPixelBufferAdaptorWithAssetWriterInput:input sourcePixelBufferAttributes:@{(NSString*)kCVPixelBufferPixelFormatTypeKey:@(kCVPixelFormatType_32ARGB),(NSString*)kCVPixelBufferWidthKey:@64,(NSString*)kCVPixelBufferHeightKey:@64}];
    [writer addInput:input];Check([writer startWriting],@"writer start");[writer startSessionAtSourceTime:kCMTimeZero];
    CVPixelBufferRef pixel=NULL;Check(CVPixelBufferPoolCreatePixelBuffer(NULL,adaptor.pixelBufferPool,&pixel)==kCVReturnSuccess,@"pixel");
    CVPixelBufferLockBaseAddress(pixel,0);memset(CVPixelBufferGetBaseAddress(pixel),0x77,CVPixelBufferGetDataSize(pixel));CVPixelBufferUnlockBaseAddress(pixel,0);
    dispatch_semaphore_t done=dispatch_semaphore_create(0);__block int frame=0;__block BOOL ended=NO;
    [input requestMediaDataWhenReadyOnQueue:dispatch_queue_create("media.test",DISPATCH_QUEUE_SERIAL) usingBlock:^{
        if(ended)return;
        while(input.readyForMoreMediaData && frame<2) { Check([adaptor appendPixelBuffer:pixel withPresentationTime:CMTimeMake(frame++,1)],@"append"); }
        if(frame==2 && !ended){ended=YES;CVPixelBufferRelease(pixel);[input markAsFinished];[writer endSessionAtSourceTime:CMTimeMake(2,1)];[writer finishWritingWithCompletionHandler:^{dispatch_semaphore_signal(done);}];}
    }];
    Check(dispatch_semaphore_wait(done,dispatch_time(DISPATCH_TIME_NOW,60*NSEC_PER_SEC))==0,@"writer timeout");Check(writer.status==AVAssetWriterStatusCompleted,@"cover writer");
    AVURLAsset *v=[AVURLAsset URLAssetWithURL:video options:nil],*a=[AVURLAsset URLAssetWithURL:audio options:nil];
    Check([v tracksWithMediaType:AVMediaTypeAudio].count==0,@"video-only fixture detected");
    AVMutableComposition *composition=[AVMutableComposition composition];
    AVMutableCompositionTrack *vt=[composition addMutableTrackWithMediaType:AVMediaTypeVideo preferredTrackID:kCMPersistentTrackID_Invalid],*at=[composition addMutableTrackWithMediaType:AVMediaTypeAudio preferredTrackID:kCMPersistentTrackID_Invalid];
    Check([vt insertTimeRange:CMTimeRangeMake(kCMTimeZero,v.duration) ofTrack:[v tracksWithMediaType:AVMediaTypeVideo].firstObject atTime:kCMTimeZero error:&error],@"video insert");
    Check([at insertTimeRange:CMTimeRangeMake(kCMTimeZero,v.duration) ofTrack:[a tracksWithMediaType:AVMediaTypeAudio].firstObject atTime:kCMTimeZero error:&error],@"audio insert");
    Export(composition,muxed,AVAssetExportPresetPassthrough,AVFileTypeMPEG4);
    AVURLAsset *out=[AVURLAsset URLAssetWithURL:muxed options:nil];
    Check([out tracksWithMediaType:AVMediaTypeVideo].count==1 && [out tracksWithMediaType:AVMediaTypeAudio].count==1,@"mux has audio+video");
    Check(fabs(CMTimeGetSeconds(out.duration)-2)<.05,@"full duration");
    Check([a tracksWithMediaType:AVMediaTypeAudio].count==1,@"shareable m4a");
    for(NSURL *url in @[wav,video,audio,muxed])[NSFileManager.defaultManager removeItemAtURL:url error:nil];
    NSLog(@"PASS silent-stream detection, static-cover generation, M4A, passthrough mux audio+video and duration");
} return 0; }
