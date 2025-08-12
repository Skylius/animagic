#include "DisassembleWorker.h"

void DisassembleWorker::start(){
    try{
        MagickBackend be;
        be.extractFrames(input, outDir, "frame_%05d.png", "PNG", true,
            [this](int d, int t){ emit progress(d, t); });
        emit done(true, QObject::tr("Extracted frames."));
    }catch(const std::exception& e){
        emit done(false, QString::fromUtf8(e.what()));
    }
}
