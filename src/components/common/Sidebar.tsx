import React from 'react';
import {
  LayoutDashboard,
  Trophy,
  Users,
  Calendar,
  Settings,
} from 'lucide-react';

export type NavItem =
  | 'dashboard'
  | 'competitions'
  | 'matches'
  | 'teams'
  | 'settings';

interface SidebarProps {
  currentTab: NavItem;
  onSelectTab: (tab: NavItem) => void;
  onCreateCompetition: () => void;
  onLogoClick?: () => void;
}

export const Sidebar: React.FC<SidebarProps> = ({
  currentTab,
  onSelectTab,
  onCreateCompetition,
  onLogoClick,
}) => {
  const NAV_ITEMS: { id: NavItem; label: string; icon: React.ReactNode }[] = [
    {
      id: 'dashboard',
      label: 'Dashboard',
      icon: <LayoutDashboard className="w-4 h-4" />,
    },
    {
      id: 'competitions',
      label: 'Competitions',
      icon: <Trophy className="w-4 h-4" />,
    },
    {
      id: 'matches',
      label: 'Matches',
      icon: <Calendar className="w-4 h-4" />,
    },
    {
      id: 'teams',
      label: 'Teams',
      icon: <Users className="w-4 h-4" />,
    },
    {
      id: 'settings',
      label: 'Settings',
      icon: <Settings className="w-4 h-4" />,
    },
  ];

  return (
    <aside className="w-60 bg-[#12151a] border-r border-[#D7E2EA]/15 flex flex-col justify-between h-screen sticky top-0 select-none z-30">
      <div>
        {/* Brand Header — Clicking logo returns to Landing page */}
        <div className="p-5 border-b border-[#D7E2EA]/15">
          <button
            type="button"
            onClick={onLogoClick}
            className="flex items-center gap-2.5 text-left w-full group cursor-pointer"
            title="Return to MatchMint Landing Page"
          >
            <div className="w-8 h-8 rounded-xl bg-gradient-to-br from-[#B600A8] to-[#7621B0] flex items-center justify-center text-white shadow-[0_2px_10px_rgba(182,0,168,0.3)] group-hover:scale-105 transition-transform">
              <span className="font-bold font-mono text-sm">M</span>
            </div>
            <div>
              <div className="flex items-center gap-1.5">
                <span className="font-bold text-base tracking-tight text-white group-hover:text-[#D7E2EA] transition-colors">
                  MatchMint
                </span>
                <span className="w-1.5 h-1.5 rounded-full bg-[#B600A8]" />
              </div>
              <p className="text-[11px] font-mono text-[#D7E2EA]/60 leading-tight">
                Tournament Manager
              </p>
            </div>
          </button>
        </div>

        {/* Navigation Items */}
        <nav className="p-3 space-y-1">
          {NAV_ITEMS.map((item) => {
            const isActive = currentTab === item.id;
            return (
              <button
                key={item.id}
                onClick={() => onSelectTab(item.id)}
                className={`w-full flex items-center gap-2.5 px-3 py-2 rounded-lg text-xs font-medium transition-all ${
                  isActive
                    ? 'bg-[#171a21] text-white border-l-4 border-l-[#B600A8] border-y border-r border-[#D7E2EA]/15 shadow-sm font-semibold'
                    : 'text-[#D7E2EA]/70 hover:text-white hover:bg-[#171a21]'
                }`}
              >
                <span className={isActive ? 'text-[#B600A8]' : 'text-[#D7E2EA]/60'}>
                  {item.icon}
                </span>
                <span>{item.label}</span>
              </button>
            );
          })}
        </nav>

        {/* Action Button */}
        <div className="px-4 py-2">
          <button
            onClick={onCreateCompetition}
            style={{
              background: 'linear-gradient(123deg, #18011F 7%, #B600A8 37%, #7621B0 72%, #BE4C00 100%)',
              boxShadow: '0px 4px 4px rgba(181, 1, 167, 0.25), 4px 4px 12px #7721B1 inset',
              outline: '2px solid #ffffff',
              outlineOffset: '-3px',
            }}
            className="w-full flex items-center justify-center gap-1.5 py-2.5 px-3 rounded-full text-white font-medium uppercase tracking-wider text-xs hover:opacity-90 transition-opacity cursor-pointer"
          >
            <span>+ Create Tournament</span>
          </button>
        </div>
      </div>

      {/* Footer Profile Pill */}
      <div className="p-4 border-t border-[#D7E2EA]/15">
        <div className="flex items-center gap-2.5">
          <div className="w-8 h-8 rounded-lg bg-gradient-to-br from-[#18011F] to-[#7621B0]/40 text-white border border-[#B600A8]/40 flex items-center justify-center font-bold text-xs font-mono shrink-0">
            S
          </div>
          <div className="min-w-0">
            <div className="text-xs font-semibold text-white truncate">
              Campus Admin
            </div>
            <div className="text-[10px] text-[#D7E2EA]/60 truncate font-mono">
              Organizer Mode
            </div>
          </div>
        </div>
      </div>
    </aside>
  );
};
