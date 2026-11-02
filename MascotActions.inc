// Native timing/pose vocabulary, researched against Shimeji-ee default conf/actions.xml.
// Reference only: https://github.com/TigerHix/shimeji-ee/blob/master/conf/actions.xml
// No upstream engine, expressions, classes or scripts are evaluated.
static NSDictionary *TTDefaultPetActions(void) {
    static NSDictionary *map;static dispatch_once_t once;dispatch_once(&once, ^{map=@{
@"stand":@[@[@1,@250,@64,@128]],
@"walk":@[@[@1,@6,@64,@128],@[@2,@6,@64,@128],@[@1,@6,@64,@128],@[@3,@6,@64,@128]],
@"run":@[@[@1,@2,@64,@128],@[@2,@2,@64,@128],@[@1,@2,@64,@128],@[@3,@2,@64,@128]],
@"dash":@[@[@1,@2,@64,@128],@[@2,@2,@64,@128],@[@1,@2,@64,@128],@[@3,@2,@64,@128]],
@"sit":@[@[@11,@250,@64,@128]],
@"lookup":@[@[@26,@250,@64,@128]],
@"dangle":@[@[@31,@5,@64,@112],@[@32,@15,@64,@112],@[@31,@5,@64,@112],@[@33,@15,@64,@112]],
@"lie":@[@[@21,@250,@64,@128]],
@"crawl":@[@[@20,@28,@64,@128],@[@20,@4,@64,@128],@[@21,@4,@64,@128],@[@21,@4,@64,@128],@[@21,@24,@64,@128]],
@"jump":@[@[@22,@250,@64,@128]],
@"fall":@[@[@4,@250,@64,@128]],
@"bounce":@[@[@18,@4,@64,@128],@[@19,@4,@64,@128]],
@"climb":@[@[@14,@16,@64,@128],@[@14,@4,@64,@128],@[@12,@4,@64,@128],@[@13,@4,@64,@128],@[@13,@16,@64,@128],@[@13,@4,@64,@128],@[@12,@4,@64,@128],@[@14,@4,@64,@128]],
@"ceiling":@[@[@25,@16,@64,@48],@[@25,@4,@64,@48],@[@23,@4,@64,@48],@[@24,@4,@64,@48],@[@24,@16,@64,@48],@[@24,@4,@64,@48],@[@23,@4,@64,@48],@[@25,@4,@64,@48]],
@"hang":@[@[@23,@250,@64,@48]],
@"grab":@[@[@13,@250,@64,@128]],
@"drag":@[@[@9,@5,@64,@128],@[@7,@5,@64,@128],@[@1,@5,@64,@128],@[@8,@5,@64,@128],@[@10,@5,@64,@128]],
@"thrown":@[@[@4,@250,@64,@128]],
@"spin":@[@[@26,@5,@64,@128],@[@15,@5,@64,@128],@[@27,@5,@64,@128],@[@16,@5,@64,@128],@[@28,@5,@64,@128],@[@17,@5,@64,@128],@[@29,@5,@64,@128],@[@11,@5,@64,@128]],
@"sprawl":@[@[@21,@250,@64,@128]],
@"trip":@[@[@19,@8,@64,@128],@[@18,@4,@64,@128],@[@20,@4,@64,@128],@[@20,@10,@64,@128],@[@19,@4,@64,@104]],
@"wallsit":@[@[@13,@250,@64,@128]],
@"legsUp":@[@[@30,@250,@64,@112]],
@"legsDown":@[@[@31,@250,@64,@112]]
};});return map;
}
static NSDictionary *TTPetActionAliases(void) {return @{@"Stand":@"stand",@"Walk":@"walk",@"Run":@"run",@"Dash":@"dash",@"Sit":@"sit",@"SitAndLookUp":@"lookup",@"SitAndDangleLegs":@"dangle",@"Creep":@"crawl",@"Jumping":@"jump",@"Falling":@"fall",@"Bouncing":@"bounce",@"ClimbWall":@"climb",@"ClimbCeiling":@"ceiling",@"GrabCeiling":@"hang",@"GrabWall":@"grab",@"Pinched":@"drag",@"SitAndSpinHeadAction":@"spin",@"Sprawl":@"sprawl",@"Tripping":@"trip",@"SitWithLegsUp":@"legsUp",@"SitWithLegsDown":@"legsDown"};}
