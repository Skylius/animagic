#include <cmath>
#include "MagickBackend.h"
#include <Magick++.h>
#include <QDir>
#include <QFileInfo>
#include <vector>
#include <stdexcept>

static inline int msToCentiseconds(int ms){
    if(ms<=0) return 0;
    int cs = (ms+5)/10;
    if(cs<1) cs=1;
    return cs;
}

MagickBackend::MagickBackend(Mode){ Magick::InitializeMagick(nullptr); }

AnimationProbe MagickBackend::probeAnimation(const QString& path){
    std::vector<Magick::Image> imgs;
    Magick::readImages(&imgs, path.toStdString());
    AnimationProbe p;
    if(!imgs.empty()){
        p.width = imgs[0].columns();
        p.height = imgs[0].rows();
        p.frames = (int)imgs.size();
        p.loop = imgs[0].animationIterations();
    }
    return p;
}

void MagickBackend::extractFrames(const QString& path, const QString& outFolder, const QString& pattern, const QString& stillFormat, bool coalesce, std::function<void(int,int)> progress){
    std::vector<Magick::Image> imgs;
    Magick::readImages(&imgs, path.toStdString());
    if(imgs.empty()) throw std::runtime_error("No frames");
    if(coalesce){
        std::vector<Magick::Image> coalesced;
        Magick::coalesceImages(&coalesced, imgs.begin(), imgs.end());
        imgs.swap(coalesced);
    }
    QDir dir(outFolder);
    for(size_t i=0;i<imgs.size();++i){
        if(progress) progress(static_cast<int>(i), static_cast<int>(imgs.size()));
        char buf[64]; snprintf(buf, sizeof(buf), pattern.toStdString().c_str(), (int)i);
        QString out = dir.filePath(QString::fromUtf8(buf));
        Magick::Image im = imgs[i];
        im.magick(stillFormat.toStdString());
        im.write(out.toStdString());
    }
}

void MagickBackend::assembleGif(const QStringList& frames, const QString& outPath, int loop, std::optional<int> delayMs, std::optional<double> fps, bool optimize, bool dither, int colors, std::function<void(int,int)> progress){
    assembleGif(frames, outPath, loop, delayMs, fps, optimize, dither, colors, 0, 0, progress);
}

void MagickBackend::assembleGif(const QStringList& frames, const QString& outPath, int loop, std::optional<int> delayMs, std::optional<double> fps, bool optimize, bool dither, int colors, int width, int height, std::function<void(int,int)> progress){
    std::vector<Magick::Image> imgs; imgs.reserve(frames.size());
    for(const auto& f : frames){
        if(progress) progress(static_cast<int>(&f - &frames[0]), frames.size());
        if(progress) progress(static_cast<int>(&f - &frames[0]), frames.size());
        Magick::Image im(f.toStdString());
        if(delayMs) im.animationDelay(msToCentiseconds(*delayMs));
        imgs.emplace_back(std::move(im));
    }
    for(auto& im: imgs) im.animationIterations(loop);
    if(optimize){
        std::vector<Magick::Image> coalesced;
        Magick::coalesceImages(&coalesced, imgs.begin(), imgs.end());
        Magick::optimizeTransparency(coalesced.begin(), coalesced.end());
        imgs.swap(coalesced);
    }
    for(auto& im: imgs){
        im.quantizeDither(dither);
        im.quantizeColors(colors);
        im.magick("GIF");
    }
    for(size_t i=0;i<imgs.size();++i){ if(progress) progress(static_cast<int>(i), static_cast<int>(imgs.size())); }
    Magick::writeImages(imgs.begin(), imgs.end(), outPath.toStdString());
}

void MagickBackend::assembleWebp(const QStringList& frames, const QString& outPath, int loop, std::optional<int> delayMs, std::optional<double> fps, bool lossless, int quality, int method, int nearLossless, int alphaQuality, std::function<void(int,int)> progress){
    assembleWebp(frames, outPath, loop, delayMs, fps, lossless, quality, method, nearLossless, alphaQuality, 0, 0, progress);
}

void MagickBackend::assembleWebp(const QStringList& frames, const QString& outPath, int loop, std::optional<int> delayMs, std::optional<double> fps, bool lossless, int quality, int method, int nearLossless, int alphaQuality, int width, int height, std::function<void(int,int)> progress){
    std::vector<Magick::Image> imgs; imgs.reserve(frames.size());
    for(const auto& f : frames){
        if(progress) progress(static_cast<int>(&f - &frames[0]), frames.size());
        if(progress) progress(static_cast<int>(&f - &frames[0]), frames.size());
        Magick::Image im(f.toStdString());
        if(delayMs) im.animationDelay(msToCentiseconds(*delayMs));
        imgs.emplace_back(std::move(im));
    }
    for(auto& im: imgs) im.animationIterations(loop);
    if(lossless){
        for(auto& im: imgs){ im.defineValue("webp","lossless","true"); }
    }else{
        for(auto& im: imgs){ im.quality(quality); }
    }
    for(auto& im: imgs){
        im.defineValue("webp","method", std::to_string(method));
        im.defineValue("webp","alpha-quality", std::to_string(alphaQuality));
        if(nearLossless>0) im.defineValue("webp","near-lossless", std::to_string(nearLossless));
        im.magick("WEBP");
    }
    for(size_t i=0;i<imgs.size();++i){ if(progress) progress(static_cast<int>(i), static_cast<int>(imgs.size())); }
    Magick::writeImages(imgs.begin(), imgs.end(), outPath.toStdString());
}

// overload forwarding

void MagickBackend::extractFrames(const QString& path, const QString& outFolder, const QString& pattern, const QString& stillFormat, bool coalesce){
    extractFrames(path, outFolder, pattern, stillFormat, coalesce, {});
}
void MagickBackend::assembleGif(const QStringList& frames, const QString& outPath, int loop, std::optional<int> delayMs, std::optional<double> fps, bool optimize, bool dither, int colors){
    assembleGif(frames, outPath, loop, delayMs, fps, optimize, dither, colors, {});
}
void MagickBackend::assembleWebp(const QStringList& frames, const QString& outPath, int loop, std::optional<int> delayMs, std::optional<double> fps, bool lossless, int quality, int method, int nearLossless, int alphaQuality){
    assembleWebp(frames, outPath, loop, delayMs, fps, lossless, quality, method, nearLossless, alphaQuality, {});
}
