import React, { useRef } from 'react';
import { motion, useScroll, useTransform, useReducedMotion, MotionValue } from 'framer-motion';

interface AnimatedTextProps {
  text: string;
  className?: string;
}

interface CharSpanProps {
  char: string;
  progress: MotionValue<number>;
  range: [number, number];
}

const CharSpan: React.FC<CharSpanProps> = ({ char, progress, range }) => {
  const opacity = useTransform(progress, range, [0.2, 1]);

  return (
    <span className="relative inline-block">
      <span className="opacity-0 select-none">{char}</span>
      <motion.span style={{ opacity }} className="absolute inset-0">
        {char}
      </motion.span>
    </span>
  );
};

export const AnimatedText: React.FC<AnimatedTextProps> = ({ text, className = '' }) => {
  const containerRef = useRef<HTMLParagraphElement>(null);
  const shouldReduceMotion = useReducedMotion();

  const { scrollYProgress } = useScroll({
    target: containerRef,
    offset: ['start 0.8', 'end 0.2'],
  });

  if (shouldReduceMotion) {
    return <p className={className}>{text}</p>;
  }

  // Split into words, then characters to ensure natural wrapping
  const words = text.split(' ');
  let charCounter = 0;
  const totalChars = text.length;

  return (
    <p ref={containerRef} className={className}>
      {words.map((word, wordIndex) => {
        const wordChars = word.split('');
        const renderedWord = (
          <span key={wordIndex} className="inline-block whitespace-nowrap">
            {wordChars.map((char, charIndex) => {
              const charIdx = charCounter++;
              const start = charIdx / totalChars;
              const end = Math.min(1, (charIdx + 1) / totalChars);
              return (
                <CharSpan
                  key={charIndex}
                  char={char}
                  progress={scrollYProgress}
                  range={[start, end]}
                />
              );
            })}
            {wordIndex < words.length - 1 && (
              <span className="inline-block">&nbsp;</span>
            )}
          </span>
        );
        charCounter++; // account for space
        return renderedWord;
      })}
    </p>
  );
};

