#include "AssembleWorker.h"

void AssembleWorker::start(){
    try{
        MagickBackend be;
        if(toWebp){
            std::optional<int> delayOpt = (fps>0.0) ? std::optional<int>() : std::optional<int>(delay);
            std::optional<double> fpsOpt = (fps>0.0) ? std::optional<double>(fps) : std::optional<double>();
            be.assembleWebp(frames, outPath, loop, delayOpt, fpsOpt, lossless, quality, method, 0, 100, width, height,
                            [this](int d,int t){ emit progress(d,t); });
        }else{
            std::optional<int> delayOpt = (fps>0.0) ? std::optional<int>() : std::optional<int>(delay);
            std::optional<double> fpsOpt = (fps>0.0) ? std::optional<double>(fps) : std::optional<double>();
            be.assembleGif(frames, outPath, loop, delayOpt, fpsOpt, optimize, dither, colors,
                           width, height,
                           [this](int d,int t){ emit progress(d,t); });
        }
        emit done(true, QObject::tr("Wrote %1").arg(outPath));
    }catch(const std::exception& e){
        emit done(false, QString::fromUtf8(e.what()));
    }
}
