import { BoardSquares, ColorGroups, TileTypes, getTileGridCoords } from '../engine/boardData.js';

export class TwoDBoard {
  constructor(containerEl, onTileClick = null) {
    this.container = containerEl;
    this.onTileClick = onTileClick;
    this.tileEls = [];
    this.playerPins = [];

    this.render();
  }

  render() {
    this.container.innerHTML = `
      <div class="board-2d-wrapper">
        <div class="board-2d-grid" id="board-2d-grid">
          <!-- 40 perimeter tiles + center area -->
          <div class="board-2d-center">
            <div class="center-banner">
              <span class="center-title">MONOPOLY AI</span>
              <span class="center-subtitle">EXPECTIMINIMAX ENGINE</span>
            </div>
            <div class="center-decks">
              <div class="deck-box chance-deck">CHANCE</div>
              <div class="deck-box cc-deck">COMMUNITY CHEST</div>
            </div>
          </div>
        </div>
      </div>
    `;

    const grid = document.getElementById('board-2d-grid');
    this.tileEls = [];

    // Create 40 tiles and position them on the 11x11 grid using CSS grid-column and grid-row
    for (let i = 0; i < 40; i++) {
      const sq = BoardSquares[i];
      const coords = getTileGridCoords(i);
      // CSS grid is 1-indexed (row 1..11, col 1..11)
      const gridRow = coords.row + 1;
      const gridCol = coords.col + 1;

      const tileEl = document.createElement('div');
      tileEl.className = `tile-2d tile-${coords.side} ${sq.type}`;
      tileEl.style.gridRow = `${gridRow}`;
      tileEl.style.gridColumn = `${gridCol}`;
      tileEl.setAttribute('data-tile', i);

      const group = ColorGroups[sq.group?.toUpperCase()];
      const bannerColor = group ? group.color : null;

      let innerHtml = '';
      if (sq.type === TileTypes.PROPERTY && bannerColor) {
        innerHtml = `
          <div class="tile-2d-color-bar" style="background:${bannerColor}"></div>
          <div class="tile-2d-content">
            <div class="tile-2d-name">${sq.name}</div>
            <div class="tile-2d-houses" id="tile-2d-houses-${i}"></div>
            <div class="tile-2d-price">$${sq.price}</div>
          </div>
        `;
      } else {
        innerHtml = `
          <div class="tile-2d-content corner-or-special">
            <div class="tile-2d-name">${sq.name}</div>
            ${sq.type === TileTypes.RAILROAD ? '<div class="tile-icon">🚂 $200</div>' : ''}
            ${sq.type === TileTypes.UTILITY ? `<div class="tile-icon">${i === 12 ? '💡' : '🚰'} $150</div>` : ''}
            ${sq.type === TileTypes.TAX ? `<div class="tile-icon">💰 $${sq.taxAmount}</div>` : ''}
          </div>
        `;
      }

      innerHtml += `<div class="tile-2d-tokens" id="tile-2d-tokens-${i}"></div>`;
      tileEl.innerHTML = innerHtml;

      tileEl.addEventListener('click', () => {
        if (this.onTileClick) this.onTileClick(i);
      });

      grid.appendChild(tileEl);
      this.tileEls.push(tileEl);
    }
  }

  // Update players and tokens on 2D board
  updatePlayers(players) {
    // Clear all tokens
    for (let i = 0; i < 40; i++) {
      const container = document.getElementById(`tile-2d-tokens-${i}`);
      if (container) container.innerHTML = '';
    }

    players.forEach(p => {
      if (p.bankrupt) return;
      const container = document.getElementById(`tile-2d-tokens-${p.position}`);
      if (container) {
        const pin = document.createElement('span');
        pin.className = 'player-2d-pin';
        pin.style.background = p.color;
        pin.title = p.name;
        pin.textContent = p.avatar;
        container.appendChild(pin);
      }
    });
  }

  // Update property ownership borders and houses
  updateProperties(properties, players) {
    for (const idx in properties) {
      const prop = properties[idx];
      const tileEl = this.tileEls[idx];
      if (!tileEl) continue;

      if (prop.owner !== null) {
        const owner = players[prop.owner];
        tileEl.style.boxShadow = `inset 0 0 0 3px ${owner.color}`;
      } else {
        tileEl.style.boxShadow = 'none';
      }

      // Houses
      const houseEl = document.getElementById(`tile-2d-houses-${idx}`);
      if (houseEl) {
        if (prop.hotel) {
          houseEl.innerHTML = '<span class="hotel-pill">HOTEL</span>';
        } else if (prop.houses > 0) {
          houseEl.innerHTML = '🏠'.repeat(prop.houses);
        } else {
          houseEl.innerHTML = '';
        }
      }
    }
  }
}
