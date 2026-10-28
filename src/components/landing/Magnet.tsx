import React, { useRef, useState, useEffect } from 'react';
import { useReducedMotion } from 'framer-motion';

interface MagnetProps {
  children: React.ReactNode;
  padding?: number;
  strength?: number;
  activeTransition?: string;
  inactiveTransition?: string;
  className?: string;
}

export const Magnet: React.FC<MagnetProps> = ({
  children,
  padding = 150,
  strength = 3,
  activeTransition = 'transform 0.3s ease-out',
  inactiveTransition = 'transform 0.6s ease-in-out',
  className = '',
}) => {
  const elementRef = useRef<HTMLDivElement>(null);
  const [style, setStyle] = useState<React.CSSProperties>({
    transform: 'translate3d(0px, 0px, 0px)',
    transition: inactiveTransition,
    willChange: 'transform',
  });

  const shouldReduceMotion = useReducedMotion();

  useEffect(() => {
    // Disable on touch devices or if reduced motion is requested
    const isTouch = window.matchMedia('(pointer: coarse)').matches;
    if (isTouch || shouldReduceMotion) return;

    const handleMouseMove = (e: MouseEvent) => {
      const el = elementRef.current;
      if (!el) return;

      const rect = el.getBoundingClientRect();
      const centerX = rect.left + rect.width / 2;
      const centerY = rect.top + rect.height / 2;

      const dx = e.clientX - centerX;
      const dy = e.clientY - centerY;

      const maxDistX = rect.width / 2 + padding;
      const maxDistY = rect.height / 2 + padding;

      if (Math.abs(dx) <= maxDistX && Math.abs(dy) <= maxDistY) {
        setStyle({
          transform: `translate3d(${dx / strength}px, ${dy / strength}px, 0px)`,
          transition: activeTransition,
          willChange: 'transform',
        });
      } else {
        setStyle({
          transform: 'translate3d(0px, 0px, 0px)',
          transition: inactiveTransition,
          willChange: 'transform',
        });
      }
    };

    const handleMouseLeave = () => {
      setStyle({
        transform: 'translate3d(0px, 0px, 0px)',
        transition: inactiveTransition,
        willChange: 'transform',
      });
    };

    window.addEventListener('mousemove', handleMouseMove, { passive: true });
    document.addEventListener('mouseleave', handleMouseLeave);

    return () => {
      window.removeEventListener('mousemove', handleMouseMove);
      document.removeEventListener('mouseleave', handleMouseLeave);
    };
  }, [padding, strength, activeTransition, inactiveTransition, shouldReduceMotion]);

  return (
    <div ref={elementRef} className={className} style={style}>
      {children}
    </div>
  );
};

