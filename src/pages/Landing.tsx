import React, { useRef, useEffect, useState } from 'react';
import { motion, useScroll, useTransform, useReducedMotion } from 'framer-motion';
import {
  Trophy,
  Calendar,
  Users,
  Undo2,
} from 'lucide-react';
import {
  FadeIn,
  Magnet,
  AnimatedText,
  PrimaryButton,
  GhostButton,
} from '../components/landing';
import { HeroVisual } from '../components/landing/HeroVisual';
import { MockupTile } from '../components/landing/MockupTiles';
import { CompetitionMediaItem } from '../components/landing/CompetitionCardMockups';
import { Competition } from '../types';

interface LandingProps {
  onOpenApp: (tab?: string, competitionId?: string) => void;
  competitions: Competition[];
}

export const Landing: React.FC<LandingProps> = ({ onOpenApp, competitions }) => {
  const shouldReduceMotion = useReducedMotion();

  // -------------------------------------------------------------
  // 1. MARQUEE SCROLL LOGIC
  // -------------------------------------------------------------
  const marqueeSectionRef = useRef<HTMLDivElement>(null);
  const [marqueeOffset, setMarqueeOffset] = useState<number>(0);

  useEffect(() => {
    if (shouldReduceMotion) return;

    let ticking = false;
    const handleScroll = () => {
      if (!ticking) {
        window.requestAnimationFrame(() => {
          if (marqueeSectionRef.current) {
            const rect = marqueeSectionRef.current.getBoundingClientRect();
            const sectionTop = rect.top + window.scrollY;
            const offset = (window.scrollY - sectionTop + window.innerHeight) * 0.3;
            setMarqueeOffset(offset);
          }
          ticking = false;
        });
        ticking = true;
      }
    };

    window.addEventListener('scroll', handleScroll, { passive: true });
    handleScroll();

    return () => window.removeEventListener('scroll', handleScroll);
  }, [shouldReduceMotion]);

  // -------------------------------------------------------------
  // 2. COMPETITION STACKING CARDS DATA
  // -------------------------------------------------------------
  const sampleComps = [
    {
      id: 'comp-gehu-2026',
      number: '01',
      name: 'GEHU Winter Cup 2026',
      category: 'Sports' as const,
      formatDesc: '8-team single elimination knockout',
      slug: 'gehu-winter-cup-2026',
    },
    {
      id: 'comp-cyberclash',
      number: '02',
      name: 'CyberClash Masters',
      category: 'Esports' as const,
      formatDesc: '8-team single elimination knockout',
      slug: 'cyberclash-masters',
    },
    {
      id: 'comp-inter-league',
      number: '03',
      name: 'Inter-College League',
      category: 'Sports' as const,
      formatDesc: '6-team round robin tournament',
      slug: 'inter-college-league',
    },
  ];

  // Smooth scroll helper
  const scrollToSection = (id: string) => {
    const el = document.getElementById(id);
    if (el) {
      el.scrollIntoView({ behavior: 'smooth' });
    }
  };

  return (
    <div className="main-wrapper text-[#D7E2EA] bg-[#0C0C0C]">
      {/* ============================================================== */}
      {/* 1. HERO SECTION                                                */}
      {/* ============================================================== */}
      <section className="h-screen flex flex-col justify-between overflow-x-clip relative">
        {/* Navbar */}
        <FadeIn delay={0} y={-20} className="w-full z-30">
          <header className="flex items-center justify-between px-6 md:px-10 pt-6 md:pt-8 text-sm md:text-lg lg:text-[1.4rem] font-medium uppercase tracking-wider text-[#D7E2EA]">
            <button
              onClick={() => scrollToSection('features')}
              className="hover:opacity-70 transition duration-200 cursor-pointer"
            >
              Features
            </button>
            <button
              onClick={() => scrollToSection('formats')}
              className="hover:opacity-70 transition duration-200 cursor-pointer"
            >
              Formats
            </button>
            <button
              onClick={() => scrollToSection('competitions')}
              className="hover:opacity-70 transition duration-200 cursor-pointer"
            >
              Competitions
            </button>
            <button
              onClick={() => onOpenApp()}
              className="hover:opacity-70 transition duration-200 cursor-pointer font-semibold text-white"
            >
              Open App
            </button>
          </header>
        </FadeIn>

        {/* Giant Title */}
        <div className="w-full overflow-hidden text-center z-10 pointer-events-none mt-6 sm:mt-4 md:-mt-5">
          <FadeIn delay={0.15} y={40}>
            <h1 className="hero-heading font-black uppercase tracking-tight leading-none whitespace-nowrap w-full text-[14vw] sm:text-[15vw] md:text-[16vw] lg:text-[17.5vw] select-none">
              MatchMint
            </h1>
          </FadeIn>
        </div>

        {/* Hero Visual Centered with Magnet */}
        <FadeIn
          delay={0.6}
          y={30}
          className="absolute left-1/2 -translate-x-1/2 z-10 w-[280px] sm:w-[360px] md:w-[440px] lg:w-[520px] top-1/2 -translate-y-1/2 sm:top-auto sm:translate-y-0 sm:bottom-0"
        >
          <Magnet padding={150} strength={3}>
            <HeroVisual />
          </Magnet>
        </FadeIn>

        {/* Bottom Bar */}
        <div className="flex justify-between items-end pb-7 sm:pb-8 md:pb-10 px-6 md:px-10 z-20">
          <FadeIn delay={0.35} y={20}>
            <div className="uppercase font-light tracking-wide leading-snug text-[clamp(0.75rem,1.4vw,1.5rem)] max-w-[160px] sm:max-w-[220px] md:max-w-[260px] text-[#D7E2EA]">
              a tournament manager for campus sports and esports — brackets, fixtures and live standings in one place
            </div>
          </FadeIn>

          <FadeIn delay={0.5} y={20}>
            <PrimaryButton label="Open App" onClick={() => onOpenApp()} />
          </FadeIn>
        </div>
      </section>

      {/* ============================================================== */}
      {/* 2. MARQUEE SECTION                                             */}
      {/* ============================================================== */}
      <section
        ref={marqueeSectionRef}
        className="bg-[#0C0C0C] pt-24 sm:pt-32 md:pt-40 pb-10 overflow-hidden"
      >
        <div className="flex flex-col gap-3">
          {/* Row 1: moves RIGHT: translateX(offset - 200) */}
          <div
            style={{
              transform: shouldReduceMotion
                ? 'none'
                : `translate3d(${marqueeOffset - 200}px, 0px, 0px)`,
              willChange: 'transform',
            }}
            className="flex gap-3 whitespace-nowrap"
          >
            {[...Array(3)].map((_, loopIdx) => (
              <React.Fragment key={`row1-loop-${loopIdx}`}>
                <MockupTile
                  id={`tile-1-${loopIdx}`}
                  imageSrc="/marquee/bracket.png"
                  type="bracket"
                  title="Single Elimination Tree"
                />
                <MockupTile
                  id={`tile-2-${loopIdx}`}
                  imageSrc="/marquee/standings.png"
                  type="standings"
                  title="Live Points Standings"
                />
                <MockupTile
                  id={`tile-3-${loopIdx}`}
                  imageSrc="/marquee/match.png"
                  type="match"
                  title="Today's Showdown"
                />
              </React.Fragment>
            ))}
          </div>

          {/* Row 2: moves LEFT: translateX(-(offset - 200)) */}
          <div
            style={{
              transform: shouldReduceMotion
                ? 'none'
                : `translate3d(${-(marqueeOffset - 200)}px, 0px, 0px)`,
              willChange: 'transform',
            }}
            className="flex gap-3 whitespace-nowrap"
          >
            {[...Array(3)].map((_, loopIdx) => (
              <React.Fragment key={`row2-loop-${loopIdx}`}>
                <MockupTile
                  id={`tile-4-${loopIdx}`}
                  imageSrc="/marquee/score-modal.png"
                  type="modal"
                  title="Result Auto-Slotted"
                />
                <MockupTile
                  id={`tile-5-${loopIdx}`}
                  imageSrc="/marquee/fixtures.png"
                  type="fixtures"
                  title="Round Robin Schedule"
                />
                <MockupTile
                  id={`tile-6-${loopIdx}`}
                  imageSrc="/marquee/teams.png"
                  type="teams"
                  title="Squad Rosters"
                />
              </React.Fragment>
            ))}
          </div>
        </div>
      </section>

      {/* ============================================================== */}
      {/* 3. ABOUT SECTION ("Why MatchMint")                             */}
      {/* ============================================================== */}
      <section
        id="features"
        className="min-h-screen flex flex-col items-center justify-center px-5 sm:px-8 md:px-10 py-20 relative overflow-hidden text-center"
      >
        {/* Floating Tile 1: Top Left - Trophy */}
        <FadeIn
          delay={0.1}
          duration={0.9}
          x={-80}
          y={0}
          className="absolute top-[4%] left-[1%] sm:left-[2%] md:left-[4%] pointer-events-none z-10"
        >
          <motion.div
            animate={
              shouldReduceMotion
                ? {}
                : {
                    y: [-8, 8, -8],
                    transition: { duration: 6, repeat: Infinity, ease: 'easeInOut' },
                  }
            }
            className="w-[120px] h-[120px] sm:w-[160px] sm:h-[160px] md:w-[210px] md:h-[210px] rounded-3xl border border-[#D7E2EA]/20 bg-gradient-to-br from-[#18011F]/80 via-[#12151a]/90 to-[#7621B0]/30 shadow-[0_10px_30px_rgba(118,33,176,0.25)] backdrop-blur-md flex items-center justify-center p-4"
          >
            <Trophy className="w-12 h-12 sm:w-16 sm:h-16 md:w-20 md:h-20 text-[#D7E2EA] stroke-[1.5]" />
          </motion.div>
        </FadeIn>

        {/* Floating Tile 2: Top Right - Users */}
        <FadeIn
          delay={0.15}
          duration={0.9}
          x={80}
          y={0}
          className="absolute top-[4%] right-[1%] sm:right-[2%] md:right-[4%] pointer-events-none z-10"
        >
          <motion.div
            animate={
              shouldReduceMotion
                ? {}
                : {
                    y: [8, -8, 8],
                    transition: { duration: 6.5, repeat: Infinity, ease: 'easeInOut', delay: 0.5 },
                  }
            }
            className="w-[120px] h-[120px] sm:w-[160px] sm:h-[160px] md:w-[210px] md:h-[210px] rounded-3xl border border-[#D7E2EA]/20 bg-gradient-to-br from-[#18011F]/80 via-[#12151a]/90 to-[#7621B0]/30 shadow-[0_10px_30px_rgba(118,33,176,0.25)] backdrop-blur-md flex items-center justify-center p-4"
          >
            <Users className="w-12 h-12 sm:w-16 sm:h-16 md:w-20 md:h-20 text-[#D7E2EA] stroke-[1.5]" />
          </motion.div>
        </FadeIn>

        {/* Floating Tile 3: Bottom Left - Calendar */}
        <FadeIn
          delay={0.25}
          duration={0.9}
          x={-80}
          y={0}
          className="absolute bottom-[8%] left-[3%] sm:left-[6%] md:left-[10%] pointer-events-none z-10"
        >
          <motion.div
            animate={
              shouldReduceMotion
                ? {}
                : {
                    y: [-6, 6, -6],
                    transition: { duration: 5.5, repeat: Infinity, ease: 'easeInOut', delay: 1 },
                  }
            }
            className="w-[100px] h-[100px] sm:w-[140px] sm:h-[140px] md:w-[180px] md:h-[180px] rounded-3xl border border-[#D7E2EA]/20 bg-gradient-to-br from-[#18011F]/80 via-[#12151a]/90 to-[#7621B0]/30 shadow-[0_10px_30px_rgba(118,33,176,0.25)] backdrop-blur-md flex items-center justify-center p-3"
          >
            <Calendar className="w-10 h-10 sm:w-14 sm:h-14 md:w-16 md:h-16 text-[#D7E2EA] stroke-[1.5]" />
          </motion.div>
        </FadeIn>

        {/* Floating Tile 4: Bottom Right - Undo2 */}
        <FadeIn
          delay={0.3}
          duration={0.9}
          x={80}
          y={0}
          className="absolute bottom-[8%] right-[3%] sm:right-[6%] md:right-[10%] pointer-events-none z-10"
        >
          <motion.div
            animate={
              shouldReduceMotion
                ? {}
                : {
                    y: [7, -7, 7],
                    transition: { duration: 6.2, repeat: Infinity, ease: 'easeInOut', delay: 1.5 },
                  }
            }
            className="w-[130px] h-[130px] sm:w-[170px] sm:h-[170px] md:w-[220px] md:h-[220px] rounded-3xl border border-[#D7E2EA]/20 bg-gradient-to-br from-[#18011F]/80 via-[#12151a]/90 to-[#7621B0]/30 shadow-[0_10px_30px_rgba(118,33,176,0.25)] backdrop-blur-md flex items-center justify-center p-4"
          >
            <Undo2 className="w-12 h-12 sm:w-16 sm:h-16 md:w-20 md:h-20 text-[#D7E2EA] stroke-[1.5]" />
          </motion.div>
        </FadeIn>

        {/* Section Content */}
        <div className="relative z-20 max-w-3xl mx-auto flex flex-col items-center">
          <FadeIn delay={0} y={40}>
            <h2 className="hero-heading font-black uppercase tracking-tight leading-none text-center text-[clamp(3rem,12vw,160px)]">
              Why MatchMint
            </h2>
          </FadeIn>

          <div className="mt-10 sm:mt-14 md:mt-16">
            <AnimatedText
              text="Running a campus tournament shouldn't mean spreadsheets and group chats. MatchMint builds your brackets, schedules matches without clashes, updates standings the moment a score is entered, and lets you undo a mistake in one click. Simple, fast, and ready for game day."
              className="text-[#D7E2EA] font-medium text-center leading-relaxed max-w-[560px] text-[clamp(1rem,2vw,1.35rem)]"
            />
          </div>

          <FadeIn delay={0.3} y={20} className="mt-16 sm:mt-20 md:mt-24">
            <PrimaryButton label="Open App" onClick={() => onOpenApp()} />
          </FadeIn>
        </div>
      </section>

      {/* ============================================================== */}
      {/* 4. FEATURES SECTION (WHITE SECTION)                            */}
      {/* ============================================================== */}
      <section
        id="formats"
        className="bg-[#FFFFFF] rounded-t-[40px] sm:rounded-t-[50px] md:rounded-t-[60px] px-5 sm:px-8 md:px-10 py-20 sm:py-24 md:py-32 text-[#0C0C0C]"
      >
        <div className="max-w-5xl mx-auto">
          <FadeIn delay={0} y={30}>
            <h2 className="text-[#0C0C0C] font-black uppercase tracking-tight leading-none text-center text-[clamp(3rem,12vw,160px)] mb-16 sm:mb-20 md:mb-28">
              Features
            </h2>
          </FadeIn>

          {/* Vertical 5-Item List */}
          <div className="divide-y divide-[rgba(12,12,12,0.15)] border-y border-[rgba(12,12,12,0.15)]">
            {[
              {
                number: '01',
                name: 'Knockout Brackets',
                desc: 'Generate single-elimination brackets with fair seeding. Winners move to the next round automatically.',
              },
              {
                number: '02',
                name: 'Round Robin',
                desc: 'Every team plays every other team exactly once, with fixtures generated in a balanced order.',
              },
              {
                number: '03',
                name: 'Smart Scheduling',
                desc: 'Assign venues and times, and get an instant warning if two matches clash.',
              },
              {
                number: '04',
                name: 'Live Standings',
                desc: 'Points, wins and rankings update the moment a result is saved.',
              },
              {
                number: '05',
                name: 'One-Click Undo',
                desc: 'Entered the wrong score? Roll back the last result and everything recalculates.',
              },
            ].map((feature, idx) => (
              <FadeIn key={feature.number} delay={idx * 0.1} y={20}>
                <div className="py-8 sm:py-10 md:py-12 flex flex-col md:flex-row md:items-baseline justify-between gap-4 md:gap-12">
                  <span className="font-black text-[clamp(3rem,10vw,140px)] text-[#0C0C0C] leading-none shrink-0 font-mono">
                    {feature.number}
                  </span>

                  <div className="space-y-2 md:space-y-3">
                    <h3 className="font-medium uppercase text-[clamp(1rem,2.2vw,2.1rem)] text-[#0C0C0C] leading-tight">
                      {feature.name}
                    </h3>
                    <p className="font-light leading-relaxed max-w-2xl text-[clamp(0.85rem,1.6vw,1.25rem)] text-[#0C0C0C] opacity-60">
                      {feature.desc}
                    </p>
                  </div>
                </div>
              </FadeIn>
            ))}
          </div>
        </div>
      </section>

      {/* ============================================================== */}
      {/* 5. COMPETITIONS SECTION                                        */}
      {/* ============================================================== */}
      <section
        id="competitions"
        className="bg-[#0C0C0C] rounded-t-[40px] sm:rounded-t-[50px] md:rounded-t-[60px] -mt-10 sm:-mt-12 md:-mt-14 z-10 relative pt-20 sm:pt-28 md:pt-36 pb-32 px-5 sm:px-8 md:px-10"
      >
        <div className="max-w-5xl mx-auto">
          <FadeIn delay={0} y={30}>
            <h2 className="hero-heading font-black uppercase tracking-tight leading-none text-center text-[clamp(3rem,12vw,160px)] mb-16 sm:mb-24">
              Competitions
            </h2>
          </FadeIn>

          {/* 3 Sticky Stacking Cards */}
          <div className="relative space-y-16 sm:space-y-20 pb-16">
            {sampleComps.map((comp, idx) => {
              const targetScale = 1 - (sampleComps.length - 1 - idx) * 0.03;

              return (
                <div
                  key={comp.id}
                  style={{
                    top: `clamp(80px, 12vh, 120px)`,
                  }}
                  className="sticky"
                >
                  <motion.div
                    style={{
                      marginTop: `${idx * 28}px`,
                    }}
                    className="rounded-[40px] sm:rounded-[50px] md:rounded-[60px] border-2 border-[#D7E2EA] bg-[#0C0C0C] p-4 sm:p-6 md:p-8 shadow-2xl transition-all"
                  >
                    {/* Top Row */}
                    <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-4 pb-6 sm:pb-8 border-b border-[#D7E2EA]/15">
                      <div className="flex items-center gap-4 sm:gap-6">
                        <span className="font-black text-[clamp(2.5rem,6vw,90px)] text-[#D7E2EA] leading-none font-mono">
                          {comp.number}
                        </span>
                        <div>
                          <div className="text-[11px] sm:text-xs font-mono uppercase tracking-wider text-[#B600A8] font-bold">
                            {comp.category} • {comp.formatDesc}
                          </div>
                          <div className="text-xl sm:text-2xl md:text-3xl font-bold uppercase tracking-tight text-white mt-0.5">
                            {comp.name}
                          </div>
                        </div>
                      </div>

                      <GhostButton
                        label="Open"
                        onClick={() => onOpenApp('competition_details', comp.id)}
                        className="self-start sm:self-auto"
                      />
                    </div>

                    {/* Bottom Row: 2-column image grid */}
                    <div className="grid grid-cols-1 md:grid-cols-12 gap-3 sm:gap-4 pt-4 sm:pt-6">
                      {/* Left Column 40% (2 stacked images) */}
                      <div className="md:col-span-5 flex flex-col gap-3 sm:gap-4">
                        <CompetitionMediaItem
                          slug={comp.slug}
                          index={1}
                          alt={`${comp.name} bracket screenshot`}
                          heightClass="h-[130px] sm:h-[180px] md:h-[220px]"
                          type="bracket"
                          compName={comp.name}
                          category={comp.category}
                        />
                        <CompetitionMediaItem
                          slug={comp.slug}
                          index={2}
                          alt={`${comp.name} standings screenshot`}
                          heightClass="h-[160px] sm:h-[220px] md:h-[300px]"
                          type="standings"
                          compName={comp.name}
                          category={comp.category}
                        />
                      </div>

                      {/* Right Column 60% (1 tall image) */}
                      <div className="md:col-span-7 h-[240px] sm:h-[350px] md:h-full min-h-[300px] md:min-h-[536px]">
                        <CompetitionMediaItem
                          slug={comp.slug}
                          index={3}
                          alt={`${comp.name} tournament overview screenshot`}
                          heightClass="h-full"
                          type="overview"
                          compName={comp.name}
                          category={comp.category}
                        />
                      </div>
                    </div>
                  </motion.div>
                </div>
              );
            })}
          </div>
        </div>
      </section>
    </div>
  );
};

