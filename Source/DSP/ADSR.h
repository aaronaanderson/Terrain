#pragma once 

#include <cmath>
#include <cassert>
#include <iostream>

namespace tp 
{
class ADSR
{
public:
    void prepare (double sr)
    {
        sampleRate = sr;
        calculateCoefficients();
    }
    void noteOn() { setPhase (Phase::ATTACK); }
    void noteOff() { if (currentValue > 0.0) setPhase (Phase::RELEASE); }
    enum class Phase 
    {
        ATTACK, 
        DECAY, 
        SUSTAIN, 
        RELEASE, 
        OFF 
    };
    double calculateNext()
    {
        switch (phase)
        {
            case Phase::ATTACK:
                currentValue = attack.offset + (currentValue * attack.coefficient);
                if (currentValue >= 1.0f || attack.numSamples <= 0.0)
                {
                    currentValue = 1.0f;
                    setPhase (Phase::DECAY);
                }
            break;
            case Phase::DECAY:
                currentValue = decay.offset + (currentValue * decay.coefficient);
                if (currentValue <= parameters.sustain || decay.numSamples <= 0.0)
                {
                    currentValue = parameters.sustain;
                    setPhase (Phase::SUSTAIN);
                }
            break;
            case Phase::SUSTAIN:
            break;
            case Phase::RELEASE:
                currentValue = release.offset + (currentValue * release.coefficient);
                if (currentValue <= 0.0f || release.numSamples <= 0.0)
                {
                    currentValue = 0.0f;
                    setPhase (Phase::OFF);
                }
            break;
            case Phase::OFF:
                currentValue = 0.0f;
            break;
            default:
            assert (false);
        }
        return currentValue;
    }
    struct Parameters
    {
        float attack = 200.0f;
        float decay = 50.0f;
        float sustain = 0.707f;
        float release = 400.0f;
    };

    void setAttack (float newAttack)
    {
        parameters.attack = newAttack;
        calculateAttack();
    }
    void setDecay (float newDecay)
    {
        parameters.decay = newDecay;
        calculateDecay();
    }
    void setSustain (float newSustain)
    {
        assert (newSustain >= 0.0f && newSustain <= 1.0f);
        parameters.sustain = newSustain;
        calculateDecay();
    }
    void setRelease (float newRelease)
    {
        parameters.release = newRelease;
        calculateRelease();
    }
    void setParameters (const Parameters& p)
    {
        parameters = p;
        calculateCoefficients();  
    }
    bool isActive() { return (phase == Phase::OFF) ? false : true; }

private:
    double currentValue = 0.0;
    double sampleRate;
    Parameters parameters;
    Phase phase = Phase::OFF;

    struct PhaseParameters 
    {
        double tco;
        double offset;
        double coefficient;
        double numSamples;
    };
    PhaseParameters attack;
    PhaseParameters decay;
    PhaseParameters release;

    void calculateCoefficients()
    {
        calculateAttack();
        calculateDecay();
        calculateRelease();
    }
    void calculateAttack() 
    {
        attack.numSamples = sampleRate * parameters.attack * 0.001;
        attack.tco = std::exp (-1.5);
        auto b = -std::log((1.0 + attack.tco) / attack.tco);
        attack.coefficient = std::exp (b / attack.numSamples); 
        attack.offset = (1.0 + attack.tco) * (1.0 - attack.coefficient);
    }
    void calculateDecay()
    {
        decay.numSamples = sampleRate * parameters.decay * 0.001;
        decay.tco = std::exp (-4.95);
        auto b = -std::log((1.0 + decay.tco) / decay.tco);
        decay.coefficient = std::exp (b / decay.numSamples);
        decay.offset = (parameters.sustain - decay.tco) * (1.0 - decay.coefficient);
    }
    void calculateRelease()
    {
        release.numSamples = parameters.release * sampleRate * 0.001;
        release.tco = std::exp (-4.95);
        auto b = -std::log ((1.0 + release.tco) / release.tco);
        release.coefficient = std::exp (b / release.numSamples);
        release.offset = -release.tco * (1.0 - release.coefficient);
    }
    void setPhase (Phase nextPhase) { phase = nextPhase; }
};

}